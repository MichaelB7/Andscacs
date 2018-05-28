#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <chrono>
#include <thread>

#include "definicions.h"
#include "analisis.h"
#include "temps.h"
#include "debug.h"
#include "hash.h"
#include "es.h"
#include "utils.h"
#include "finals.h"
#include "avaluacio.h"
#include "moviments.h"
#ifdef RANDOM
#include "mtwist.h"
#endif

#ifdef LINUX
#include <time.h>   // nanosleep
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifdef TB
#include "tbprobe.h"
#endif

struct tss ssbase[MaxThreads + 1];  //el darrer és per sincronitzar threads

int nivellactual; //per perft i divide
Moviment darrer_moviment = 0;
int color_que_juga_pc;  //es necessita pel contemp
char infostring[1024];
int puntuacio_final_ant_root, puntuacio_final_ant2_root, puntuacio_final_root;
bool printfpensar, posathistoricobertures = false;
int64_t nodes_mirar_temps = 2000, seguent_mirar_temps;
int canviadajugadabona, nummovimentsroot;
tss ssauxpv;
#define CadaNPrunePocImportantsBase 9

#ifdef TB
char tb_path[1024];
#endif

//refutació 2 jugades
struct EstructuraRefutacio {
	uint64_t clau;
	Moviment moviment;
};

static const int numrefutacions = 0x20000;
EstructuraRefutacio refutacio[numrefutacions];

Moviment obtenir_refutacio(ClauHash clau)
{
	const long long idx = clau & (numrefutacions - 1);
	EstructuraRefutacio tmp = { clau, Moviment(0) };
	return refutacio[idx].clau == tmp.clau ? refutacio[idx].moviment : Moviment(0);
}

void guardar_refutacio(ClauHash clau, Moviment m)
{
	const long long idx = clau & (numrefutacions - 1);
	refutacio[idx].clau = clau;
	refutacio[idx].moviment = m;
}

ClauHash obtenir_clau_refutacio(tss * RESTRICT ss)
{
	Estat_Analisis *estat_actual = ss->estat_actual;
	const Estat_Analisis *p = max((estat_actual - 2), &ss->estat[0]);
	return p->hash ^ estat_actual->hash;
}

void buidar_refutacio() {
	memset(refutacio, 0, sizeof(refutacio));
}
//fi refutació

//refutació 3 jugades
static const int numrefutacions3 = 0x40000;
EstructuraRefutacio refutacio3[numrefutacions3];

Moviment obtenir_refutacio3(ClauHash clau)
{
	if (clau == 0)
		return JugadaInvalida;
	const long long idx = clau & (numrefutacions3 - 1);
	EstructuraRefutacio tmp = { clau, Moviment(0) };
	return refutacio3[idx].clau == tmp.clau ? refutacio3[idx].moviment : Moviment(0);
}

void guardar_refutacio3(ClauHash clau, Moviment m)
{
	if (clau == 0)
		return;
	const long long idx = clau & (numrefutacions3 - 1);
	refutacio3[idx].clau = clau;
	refutacio3[idx].moviment = m;
}

ClauHash obtenir_clau_refutacio3(tss * RESTRICT ss)
{
	Estat_Analisis *estat_actual = ss->estat_actual;
	const Estat_Analisis *p2 = (estat_actual - 4);
	if (p2 < &ss->estat[0])
		return 0;
	if ((estat_actual - 3)->moviment_previ == JugadaInvalida)
		return 0;
	const Estat_Analisis *p = (estat_actual - 2);
	if (p->moviment_previ == JugadaInvalida)
		return 0;
	if ((estat_actual - 1)->moviment_previ == JugadaInvalida)
		return 0;
	if (estat_actual->moviment_previ == JugadaInvalida)
		return 0;
	return p2->hash ^ estat_actual->hash;  //és correcte, no cal p->hash doncs fent això està traient totes les jugades anteriors 
}

void buidar_refutacio3() {
	memset(refutacio3, 0, sizeof(refutacio3));
}
//fi refutació tres jugades


//hash peces
static const int numhashpeces = 0x20000;
hp_estructura hashpeces[numhashpeces];
bool temps_acabat_real;

hp_estructura * obtenir_hashpeces(ClauHash clau)
{
	const long long idx = clau & (numhashpeces - 1);
	return &hashpeces[idx];
}

void guardar_hashpeces(ClauHash clau, Moviment m)
{
	const long long idx = clau & (numhashpeces - 1);
	if (hashpeces[idx].clau == clau) {
		if (hashpeces[idx].move1 != m)
			hashpeces[idx].move2 = hashpeces[idx].move1;
	}
	else {
		hashpeces[idx].clau = clau;
		hashpeces[idx].move2 = JugadaInvalida;
	}
	hashpeces[idx].move1 = m;
}

void buidar_hashpeces() {
	memset(hashpeces, 0, sizeof(hashpeces));
}

void actualitza_countermoves(tss * RESTRICT ss, uint8_t p, uint8_t to, Moviment m) {
	if (m == ss->countermoves[p][to].primera)
		return;
	ss->countermoves[p][to].segona = ss->countermoves[p][to].primera;
	ss->countermoves[p][to].primera = m;
}

void buidar_countermoves(tss * RESTRICT ss) {
	memset(ss->countermoves, 0, sizeof(ss->countermoves));
}

#define RazorDepth 3
#define ValorEvalMargin 30
#define DepthRazoring 4 //4 //no:5
#define ValorRazoring1 105
#define ValorRazoring2 245
#define NivellFutilityPruning 7 //6
#define ValorFutilityPruning 60 //no: 230
#define ReduccioNullValorPeoActiva 0
#define ValorDeltaCutoff 140
int fmargin[3] = { 0, 120, 370 };
#define nivelllmr 2 //2
#define ExtensioMaxima 10
bool NullMove = true;
bool AlwaysFullPv = false;
int MultiPV = 1;
int Contempt = 0;
int ValorTaules[2];
int multipv;
#ifdef RANDOM
int Random = 0;
Moviment jugadarandom;
#endif

static inline int RazorMargin(int depth) {
	return 2 * vpeosee + (depth - 1)*vpeosee / 4;
}

int RazorMargin2(int depth)
{
	return 50 * depth + 50;
}

static inline int EvalMargin(int depth) {
	return depth * 3 * ValorEvalMargin;
}

void neteja_varis(tss * RESTRICT ss) {
	memset(ss->Historial, 0, sizeof(ss->Historial));
}

bool mirar_si_parar() {
	long long tempsquehapassat = temps_actual() - inici_analisis;

	if (temps_tipus & TipusTempsMoviment)
		return (tempsquehapassat > temps_TempsMoviment);
	if (tempsquehapassat > temps_maxim)
		return true;

	if (tempsquehapassat > temps_per_moure) {
		int movimentsperferx = (temps_tipus & TipusNumMoviments) ? temps_NumMoviments : movimentsperfer;

		if ((movimentsperferx > 5) &&
			(tempsquehapassat < (temps_per_moure * 2)) &&
			(temps_per_moure > 5000)) {
			return false;
		}
		else {
			return true;
		}
	}
	return false;
}

parametresroot paramroot;
void inicialitzar_threads();
int incrementthread[MaxThreads];

int numcopsavaluaciosimilar;

void mostrar_pv(tss * RESTRICT ss, int score, int alpha, int beta, int pvactual, int mirarfinsnivell) {
	char texte[256];
	char texte2[1024];
	char texte3[20];
	texte[0] = 0;
	if (score <= alpha) {
		strcpy(texte, " upperbound");
	}
	else if (score >= beta) {
		strcpy(texte, " lowerbound");
	}
	if (temps_per_moure == 0 || temps_per_moure > 1000)
		if ((texte[0] == 0 || (temps_per_moure > 10000 && mirarfinsnivell > 10)) || temps_per_moure == 0) { //lower/upper només si hi ha encara més temps

			long long h2 = temps_actual();
			long long tempsquehapassat = h2 - inici_analisis;
			if (tempsquehapassat == 0)
				tempsquehapassat = 1;

			texte3[0] = 0;
			if (multipv > 1)
				sprintf(texte3, " multipv %i", pvactual + 1);

			long long numjugadesaltresthreads = 0;
			long long tbhits = 0;
#ifdef TB
			tbhits = ssbase[0].tbhits;
#endif
			for (int i = 1; i < NumThreads; i++) {
				numjugadesaltresthreads += ssbase[i].numjugadesfetes;  //això suma encara que no hagi acabat el thread. no ve d'aquí
#ifdef TB
				tbhits += ssbase[i].tbhits;
#endif
			}
			texte2[0] = 0;
			if (score >= maxbeta - MaxProfunditat) {//mat el mòdul d'anàlisi
				sprintf(texte2, "info depth %d seldepth %d%s score mate %i nodes %lli nps %lli tbhits %lli time %lli pv ",
					mirarfinsnivell, ss->seldepth, texte3, (maxbeta - score + 2) / 2, ss->numjugadesfetes + numjugadesaltresthreads, (ss->numjugadesfetes + numjugadesaltresthreads) * 1000 / tempsquehapassat, tbhits, tempsquehapassat);
			}
			else {
				if (score <= maxalpha + MaxProfunditat) //mat el rival
					sprintf(texte2, "info depth %d seldepth %d%s score mate -%i nodes %lli nps %lli tbhits %lli time %lli pv ",
					mirarfinsnivell, ss->seldepth, texte3, (abs(maxalpha - score) + 1) / 2, ss->numjugadesfetes + numjugadesaltresthreads, (ss->numjugadesfetes + numjugadesaltresthreads) * 1000 / tempsquehapassat, tbhits, tempsquehapassat);
				else
					sprintf(texte2, "info depth %d seldepth %d%s score cp %i%s nodes %lli nps %lli tbhits %lli time %lli pv ",
					mirarfinsnivell, ss->seldepth, texte3, score, texte, ss->numjugadesfetes + numjugadesaltresthreads, (ss->numjugadesfetes + numjugadesaltresthreads) * 1000 / tempsquehapassat, tbhits, tempsquehapassat);
			}
			int y;
			for (y = 0;; y++) {
				if (texte2[y] == 0)
					break;
			}
			int longpv = 0;
			int posipv = y;
			for (int x = 0; x <= MaxProfunditat && ss->pv[0][x]; ++x) {
				moviment_a_texte_simple(ss->pv[0][x], &texte2[y]);
				if (texte2[y + 4] == 0)
					y = y + 4;
				else
					y = y + 5;
				texte2[y] = ' ';
				texte2[y + 1] = 0;
				y++;
				longpv++;
			}
			if (AlwaysFullPv && (temps_per_moure == 0 || (mirarfinsnivell > 15 && temps_per_moure > 20000))) {
				if (longpv < mirarfinsnivell / 2) {
					//recuperar el que es pugui de la pv de hash quan massa curta
					y = posipv;
					ssauxpv = ssbase[MaxThreads];
					ssauxpv.estat_actual = &ssauxpv.estat[0];
					struct thash auxhash;
					struct thash *auxhashp;
					for (int x = 0; x < mirarfinsnivell; x++) {
						auxhashp = buscar_hash(ssauxpv.estat_actual->hash);
						if (auxhashp)
							auxhash = *auxhashp;
						else
							auxhash.w1 = 0;
						if (!auxhash.w1)
							break;
						fer_moviment(&ssauxpv, movimenthash(auxhash));
						ssauxpv.estat_actual++;
						moviment_a_texte_simple(movimenthash(auxhash), &texte2[y]);
						if (texte2[y + 4] == 0)
							y = y + 4;
						else
							y = y + 5;
						texte2[y] = ' ';
						texte2[y + 1] = 0;
						y++;
					}
				}
			}
			if (printfpensar)
				afout("%s\n", texte2);
		}
}

bool mirar_si_parar_root() {
	if (tasca == tasca_cap)
		return true;
	hora_actual = temps_actual();
	if ((temps_tipus & TipusInfinit) || (temps_tipus & TipusProfunditat))
		return false;
	if (temps_tipus & TipusNodes)
		return (ssbase[0].numjugadesfetes > temps_nodes);
	int64_t tempsquehapassat = hora_actual - inici_analisis;
	if (temps_tipus & TipusTempsMoviment)
		return (tempsquehapassat > temps_TempsMoviment);
	return ((tempsquehapassat > temps_maxim) || (int64_t)((double)tempsquehapassat * (double)2.2) > temps_per_moure); //no tindrà temps per acabar el següent ply
}

void analitzar(){
	int score, alpha, beta;
	long long tempsquehapassat;
	long long hora_anterior;
	inici_analisis = hora_anterior = temps_actual();
#ifdef TB
	tss * RESTRICT ss = &ssbase[0];
	bool mostrarbestmove = true;
#ifdef TB2
	if (false && popcount(TTotespeces) <= TB_LARGEST) {
#else
	if (popcount(TTotespeces) <= TB_LARGEST) {
#endif
		unsigned res = tb_probe_root(Tpeces(blanques), Tpeces(negres),
			Trei(blanques) | Trei(negres),
			Tdames(blanques) | Tdames(negres),
			Ttorres(blanques) | Ttorres(negres),
			Talfils(blanques) | Talfils(negres),
			Tcaballs(blanques) | Tcaballs(negres),
			Tpeons(blanques) | Tpeons(negres),
			ss->estat_actual->jugades50,
			ss->estat_actual->enroc,
			ss->estat_actual->casellapeoalpas,
			(ss->estat_actual->mou == blanques), NULL);
		if (res != TB_RESULT_FAILED) {
			score = TbValues[TB_GET_WDL(res)];
			int flags = 0;
			unsigned to = TB_GET_TO(res);
			switch (TB_GET_PROMOTES(res)) {
			case TB_PROMOTES_QUEEN:
				flags |= flag_corona_dama; break;
			case TB_PROMOTES_ROOK:
				flags |= flag_corona_torre; break;
			case TB_PROMOTES_BISHOP:
				flags |= flag_corona_alfil;
			case TB_PROMOTES_KNIGHT:
				flags |= flag_corona_caball; break;
			default:
				break;
			}
			ssbase[0].MillorsMovimentsPv[0] = FerMoviment(TB_GET_FROM(res), to) | flags;
			char sstr[32];
			moviment_a_texte_simple(ssbase[0].MillorsMovimentsPv[0], sstr);
			afout("info depth 1 seldepth 1 score cp %d nodes 1 nps 0 pv %s\n", score, sstr);   // Fake PV
			afout("bestmove %s\n", sstr);
			mostrarbestmove = false;
			ssbase[0].moviment_a_ponderar = JugadaInvalida;
			tempsquehapassat = 0;
			goto fipertb;
			return;
		}
	}
#endif

	ValorTaules[ssbase[0].estat_actual->mou] = 0 - Contempt;
	ValorTaules[oponent(ssbase[0].estat_actual->mou)] = 0 + Contempt;

	temps_acabat_real = false;
	for (int i = 1; i < NumThreads; i++) {
		incrementthread[i] = i & 1;
		ssbase[i].numjugadesfetes = 0;
	}

	if (NumThreads <= 4) {
		incrementthread[0] = 0;
		incrementthread[1] = 1;
		incrementthread[2] = 2;
		incrementthread[3] = 3;
	}
	else {
		incrementthread[0] = 0;
		incrementthread[1] = 1;
		incrementthread[2] = 0;
		incrementthread[3] = 2;
		incrementthread[4] = 1;
		incrementthread[5] = 3;
		incrementthread[6] = 2;
		incrementthread[7] = 4;
	}

	if (NumThreads > 16) {
		for (int i = 16; i < NumThreads; i++) {
			incrementthread[i] = i & 7;
		}
	}

	int64_t nodes_per_segon;
	int puntuacionspv[100];
	memset(puntuacionspv, 0, sizeof(puntuacionspv));

	numcopsavaluaciosimilar = 0;
	buidar_countermoves(&ssbase[0]); //cal fer-ho sempre, sino perd algo d'elo a ltc
	temps_per_moure = 0;
	color_que_juga_pc = ssbase[0].estat_actual->mou;
	calcular_atacs(ssbase);
	ssbase[0].estat_actual->clavats = marcar_clavats(ssbase, 1, ssbase[0].estat_actual->mou);
	ssbase[0].estat_actual->descoberts = marcar_clavats(ssbase, 0, ssbase[0].estat_actual->mou);
	ssbase[0].estat_actual->clavatsrival = marcar_clavats(ssbase, 1, oponent(ssbase[0].estat_actual->mou));
	ssbase[0].numjugadesfetes = 0;
#ifdef TB
	ssbase[0].tbhits = 0;
#endif
	ssbase[0].prunepocimportants = CadaNPrunePocImportantsBase;
	ssbase[0].temps_acabat = false;
	ssbase[0].acabatroot = false;
	ssbase[0].acabatbe = false; //per marcar si és la que acava sencera
	memset(ssbase[0].MillorsMovimentsPv, JugadaInvalida, sizeof(ssbase[0].MillorsMovimentsPv));
	ssbase[0].mirarfinsnivellbase = 1;
	ssbase[0].estat_actual->reduc_lmr = 0;

	calcular_temps_moviment();
	bool usarthreads = true;
	if (temps_per_moure && temps_per_moure < 50)
		usarthreads = false;
	else
		paramroot.analitzant = true;

	if (!(temps_tipus & TipusInfinit))
		edathash++;
	puntuacio_final_ant_root = puntuacioinvalida;
	puntuacio_final_ant2_root = puntuacioinvalida;
	paramroot.alpha = maxalpha;
	paramroot.beta = maxbeta;
	paramroot.iniciar = false;
	paramroot.parar = false;
	paramroot.nointentarcomencar = true;

	if (NumThreads > 1)
		ssbase[MaxThreads] = ssbase[0];  //els altres threads es còpien això
#ifdef RANDOM
	jugadarandom = 0;
	mt_seed();
#endif
	search_root(paramroot.numthread[0], 0);
	score = ssbase[0].score;
#ifdef RANDOM
	if (!jugadarandom && (score == maxbeta || score == maxalpha + 1)) //mate ja o et fan mate en 1
		goto fianalisi;
#else
	if (score == maxbeta || score == maxalpha + 1) { //mate ja o et fan mate en 1
		if (!ssbase[0].mostradapv)
			mostrar_pv(&ssbase[0], score, 0, 0, 0, ssbase[0].mirarfinsnivellbase);
		goto fianalisi;
	}
#endif
	puntuacionspv[0] = score;
	alpha = maxalpha;
	beta = maxbeta;
	puntuacio_final_root = score;
	seguent_mirar_temps = nodes_mirar_temps;
	canviadajugadabona = 0;
	int delta; delta = 16; //per evitar error de jump amb gcc
	int threadacabat; threadacabat = 0;
	Moviment anteriorbona; anteriorbona = 0;
	int puntuacioinicial; puntuacioinicial = 9999;

	int mirarfinsprofunditat;
	mirarfinsprofunditat = MaxProfunditat;
	if (temps_tipus & TipusProfunditat)
		mirarfinsprofunditat = temps_Profunditat;

	for (ssbase[0].mirarfinsnivellbase = 2; ssbase[0].mirarfinsnivellbase <= mirarfinsprofunditat; ssbase[0].mirarfinsnivellbase++) {
		if (temps_acabat_real) {
			break;
		}

		bool b; b = mirar_si_parar_root();
		tempsquehapassat = hora_actual - hora_anterior;
		if (b) {
			if (!temps_per_moure || (temps_tipus & TipusTempsMoviment))  //no és un control de temps estàndard
				break;
			if (tempsquehapassat > temps_maxim)
				break;
			//si ja ha passat massa temps o la jugada bona no ha canviat massa cops
			if (tempsquehapassat > temps_maxim * 0.6 || canviadajugadabona <= (ssbase[0].mirarfinsnivellbase >> 1)) {
				//a més la millor jugada no ha d'haver canviat en la darrera iteració
				//ni tampoc l'avaluació ha d'estar empitjorant molt
				if (anteriorbona == ssbase[0].MillorsMovimentsPv[0] && puntuacioinicial - puntuacio_final_root < 40)
					break;
			}
			//es dona un extra de temps per inestabilitat de jugada bona
			tempsquehapassat = tempsquehapassat;
		}
		anteriorbona = ssbase[0].MillorsMovimentsPv[0];

		if (tempsquehapassat < 10000 && tempsquehapassat > 5) {
			nodes_per_segon = ssbase[0].numjugadesfetes * 1000 / (uint64_t)(tempsquehapassat);
			nodes_mirar_temps = max((nodes_per_segon / 1000) * milisegons_entre_escoltar, nodes_mirar_temps >> 1);
			if (nodes_mirar_temps > 200000)
				nodes_mirar_temps = 200000;
		}

		for (int pvactual = 0; pvactual < multipv && !temps_acabat_real; pvactual++)
		{
			// finestra aspiració
			if (ssbase[0].mirarfinsnivellbase >= 5) {
				delta = 16;
				alpha = max(puntuacionspv[pvactual] - delta, maxalpha);
				beta = min(puntuacionspv[pvactual] + delta, maxbeta);
			}
			ssbase[0].mostradapv = false;
			for (;;) {
				paramroot.alpha = alpha;
				paramroot.beta = beta;
				paramroot.pvactual = pvactual;
				ssbase[0].temps_acabat = false; //ho pot haver posat a true algun altre thread
				ssbase[MaxThreads].pararthread0 = false;
				ssbase[0].acabatroot = false;
				ssbase[0].acabatbe = false; //per marcar si és la que acava sencera
				ssbase[0].prunepocimportants = CadaNPrunePocImportantsBase;
				if (NumThreads > 1)
					ssbase[MaxThreads] = ssbase[0];  //els altres threads es còpien això
				paramroot.nointentarcomencar = false;
				if (usarthreads) {
					paramroot.iniciar = true;
					for (int i = 0; i < NumThreads; i++) {
						ssbase[i].acabatroot = false; //perquè comenci
					}
				}
				ssbase[0].temps_acabat = false; //ho pot haver posat a true algun altre thread
				search_root(paramroot.numthread[0], pvactual);
				paramroot.nointentarcomencar = true;
				threadacabat = 0;
				if (usarthreads) {
					for (int i = 1; i < NumThreads; i++) {
						if (ssbase[i].acabatbe) {
							threadacabat = i;
						}
						ssbase[i].temps_acabat = true;
					}
				}
				else
					threadacabat = 0;
				if (ssbase[0].acabatbe)
					threadacabat = 0;
				if (threadacabat != 0) {
					ssbase[0].MillorsMovimentsPv[pvactual] = ssbase[threadacabat].MillorsMovimentsPv[pvactual];
					ssbase[0].moviment_a_ponderar = ssbase[threadacabat].moviment_a_ponderar;
				}
				score = ssbase[threadacabat].score;

				if (temps_acabat_real || score == maxbeta || score == maxalpha)  //mate
					break;
				if (alpha < score && score < beta) {
					puntuacionspv[pvactual] = score;
					break;
				}
				else {
					if (score <= alpha) {
						alpha -= delta;
					}
					else if (score >= beta) {
						beta += delta;
					}
					//delta *= 2;
					//delta += delta / 2;
					delta += delta / 3 + 4;
				}
			}
			if (!temps_per_moure && !ssbase[threadacabat].mostradapv)
				mostrar_pv(&ssbase[threadacabat], score, alpha, beta, pvactual, ssbase[0].mirarfinsnivellbase);
			if (multipv == 1) { //només es fa quan no multipv
				puntuacio_final_ant2_root = puntuacio_final_ant_root;
				puntuacio_final_ant_root = puntuacio_final_root;
				puntuacio_final_root = score;
				// per fer que miri millor si realment son taules
				if (puntuacio_final_root == 0)
					neteja_varis(ssbase);
				//numcopsavaluaciosimilar
				if (abs(puntuacio_final_ant_root - score) < 2)
					numcopsavaluaciosimilar++;
				else
					numcopsavaluaciosimilar = 0;
				if (ssbase[0].mirarfinsnivellbase < 5)
					puntuacioinicial = score;
			}
			if (!((tasca == tasca_analitzar && (temps_tipus & TipusInfinit))) && (score == maxbeta || score == maxalpha || (ssbase[0].estat[0].moviments[1].moviment == JugadaInvalida && ssbase[0].mirarfinsnivellbase >= 4)))
				break; //només una jugada possible
		} //pvactual
		if (!((tasca == tasca_analitzar && (temps_tipus & TipusInfinit))) && (score == maxbeta || score == maxalpha || (ssbase[0].estat[0].moviments[1].moviment == JugadaInvalida && ssbase[0].mirarfinsnivellbase >= 4))) {
			break;  //només una jugada possible
		}
	} //mirarfinsnivellbase
fianalisi:
#ifdef RANDOM
	if (jugadarandom) {
		ssbase[0].MillorsMovimentsPv[0] = jugadarandom;
		ssbase[0].moviment_a_ponderar = 0;
	}
#endif
#ifdef LINUX
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 1000;
#endif
	paramroot.analitzant = false;
	if (usarthreads && score != maxbeta && score != maxalpha + 1) {
		for (int i = 1; i < NumThreads; i++) {
			ssbase[i].temps_acabat = true;
		}
		for (int i = 1; i < NumThreads; i++) {
			while (!ssbase[i].acabatroot) {
#ifdef WINDOWS
				Sleep(1);
#endif
#ifdef LINUX
				//nanosleep(&ts, NULL);
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
			}
		}
	}

fipertb:
	//esperar a rebre comanda per sortir si està en modo ponder o infinit
	if (tasca == tasca_ponderar || (tasca == tasca_analitzar && (temps_tipus & TipusInfinit))) {
		for (;;) {
#ifdef WINDOWS
			Sleep(1);
#endif
#ifdef LINUX
			//nanosleep(&ts, NULL);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
			if (entrada_pendent())
				comandes_uci(false);
			if (!(tasca == tasca_ponderar || (tasca == tasca_analitzar && (temps_tipus & TipusInfinit))))
				break;
		}
	}

	if (printfpensar) {
		long long numjugadesaltresthreads = 0;
#ifdef TB
		long long tbhits = 0;
#endif
		for (int i = 0; i < NumThreads; i++) {
			numjugadesaltresthreads += ssbase[i].numjugadesfetes;  //això suma encara que no hagi acabat el thread. no ve d'aquí
#ifdef TB
			tbhits += ssbase[i].tbhits;
#endif
		}
		long long h2 = temps_actual();
		tempsquehapassat = h2 - inici_analisis;
		if (tempsquehapassat == 0)
			tempsquehapassat = 1;
#ifdef TB
		afout("info nodes %lli nps %lli time %lli tbhits %lli\n",
			numjugadesaltresthreads, numjugadesaltresthreads * 1000 / tempsquehapassat, tempsquehapassat, tbhits);
#else
		afout("info nodes %lli nps %lli time %lli\n",
			numjugadesaltresthreads, numjugadesaltresthreads * 1000 / tempsquehapassat, tempsquehapassat);
#endif
	}

	// enviar el moviment a fer i el moviment a ponderar
#ifdef TB
	if (printfpensar && mostrarbestmove && (tasca == tasca_analitzar || (tasca == tasca_cap && temps_acabat_real))) {
#else
	if (tasca == tasca_analitzar || (tasca == tasca_cap && temps_acabat_real)) {
#endif
		Moviment movp = ssbase[0].moviment_a_ponderar;
		//assegurar que hi hagi un moviment a ponderar
		if (movp == JugadaInvalida){
			fer_moviment(&ssbase[0], ssbase[0].MillorsMovimentsPv[0]);
			ssbase[0].estat_actual++;
			struct thash *auxhashp;
			auxhashp = buscar_hash(ssbase[0].estat_actual->hash);
			if (auxhashp) {
				struct thash auxhash = *auxhashp;
				movp = movimenthash(auxhash);
			}
			desfer_moviment(&ssbase[0], ssbase[0].estat_actual->moviment_previ);
		}
		//mirar que el moviment a ponderar sigui legal
		if (movp != JugadaInvalida) {
			bool jugadaesescac; jugadaesescac = es_escac_bo(&ssbase[0], ssbase[0].MillorsMovimentsPv[0]);
			fer_moviment(&ssbase[0], ssbase[0].MillorsMovimentsPv[0]);
			ssbase[0].estat_actual++;
			ssbase[0].estat_actual->escac = jugadaesescac;

			calcular_atacs(&ssbase[0]);
			ssbase[0].estat_actual->clavats = marcar_clavats(&ssbase[0], 1, ssbase[0].estat_actual->mou);
			ssbase[0].estat_actual->descoberts = marcar_clavats(&ssbase[0], 0, ssbase[0].estat_actual->mou);
			ssbase[0].estat_actual->clavatsrival = Tota1BB;

			if (!jugada_legal(&ssbase[0], movp, (e_colors)ssbase[0].estat_actual->mou))
				movp = 0;
			desfer_moviment(&ssbase[0], ssbase[0].estat_actual->moviment_previ);
		}
		mostrar_moviment(ssbase[0].MillorsMovimentsPv[0], movp);
	}
	}


#ifdef WINDOWS
void proces_thread(void *param) {
#endif
#ifdef LINUX
	void* proces_thread(void *param) {
#endif
#ifdef WINDOWS
		long int numthread = *((long int*)param);
#endif
#ifdef LINUX
		long int numthread = (long int)param;
#endif
#ifdef LINUX
		struct timespec ts;
		ts.tv_sec = 0; //milliseconds / 1000;
		ts.tv_nsec = 1000; //(milliseconds % 1000) * 1000000;
#endif
		do {
			if (paramroot.analitzant && paramroot.iniciar && !ssbase[numthread].acabatroot && !paramroot.nointentarcomencar) {
				search_root(numthread, paramroot.pvactual);
				ssbase[numthread].acabatroot = true;
				if (!ssbase[numthread].temps_acabat && ssbase[numthread].pararthread0) {
					ssbase[0].temps_acabat = true; //parar thread principal
				}
				paramroot.nointentarcomencar = true;
			}
			if (paramroot.parar) {
#ifdef WINDOWS
				return;
#endif
#ifdef LINUX
				return NULL;
#endif
			}
			if (!paramroot.analitzant) {
				ssbase[numthread].acabatroot = true; //sembla necessari
				//no ocupar cpu mentre no està pensant
#ifdef WINDOWS
				Sleep(1);
#endif
#ifdef LINUX
				//nanosleep(&ts, NULL);
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
			}
		} while (true);
	}


	void search_root(long int numthread, int pvactual) {
		int score;
		MovList *ml, *movact;
		int64_t numjugadesb = 0;

		long long numjug = ssbase[numthread].numjugadesfetes;
		if (numthread != 0)
			ssbase[numthread] = ssbase[MaxThreads];  //això actualitza entre altres mirarfinsnivellbase
		ssbase[numthread].numjugadesfetes = numjug;  //no sobreescriure
		ssbase[numthread].numthread = numthread;
		ssbase[numthread].temps_acabat = false;
		ssbase[numthread].estat_actual = &ssbase[numthread].estat[0];
		ssbase[numthread].pararthread0 = false;
		if (numthread != 0) {
			ssbase[numthread].mirarfinsnivellbase += incrementthread[numthread];
		}
		tss * RESTRICT ss = &ssbase[numthread];
		int alpha = paramroot.alpha;
		int alphaori = alpha;
		int beta = paramroot.beta;
		int mirarfinsnivell = ssbase[numthread].mirarfinsnivellbase;

		ss->seldepth = 0;
		ss->moviment_a_ponderar = JugadaInvalida;
		ss->puntuacio_final = puntuacio_final_root;
		ss->puntuacio_final_ant = puntuacio_final_ant_root;
		ss->puntuacio_final_ant2 = puntuacio_final_ant2_root;
		if (numthread != 0 && paramroot.nointentarcomencar)
			return;
		//mirar si està en escac
		if (mirarfinsnivell == 1) {//generar jugades només 1r cop, sense il·legals
			const int jo = ss->estat_actual->mou;
			ss->mov = (ss->estat_actual + 1)->moviments;  //posar els moviments a estat_actual+1 per anar agafant només els legals
			if (ss->estat_actual->escac) {
				if (jo == blanques)
					generar_evasions<blanques>(ss);
				else
					generar_evasions<negres>(ss);
			}
			else {
				generar_moviments(ss);
			}
			ss->nummovimentlegal = 0;
			ml = &(ss->estat_actual + 1)->moviments[0];
			while (ml->moviment) {
				if (jugada_ilegal(ss, ml->moviment)) {
					ml++;
					continue;
				}
				ss->estat_actual->moviments[ss->nummovimentlegal] = *ml;

				//puntuació inicial segons avaluació
				fer_moviment(ss, ml->moviment);
				ss->estat_actual++;
				calcular_atacs(ss);
				ss->estat_actual->clavats = marcar_clavats(ss, 1, ss->estat_actual->mou);
				ss->estat_actual->descoberts = marcar_clavats(ss, 0, ss->estat_actual->mou);
				ss->estat_actual->clavatsrival = marcar_clavats(ss, 1, oponent(ss->estat_actual->mou));
				int punt = -avaluacio(ss); //negatiu pq és valoració de cara a qui mou després de la jugada
				if (posathistoricobertures) {
					int pecamou = ss->tau.c[Origen(ml->moviment)];
					punt += PuntuacioHistorial(pecamou, Desti(ml->moviment));
				}
				desfer_moviment(ss, ss->estat_actual->moviment_previ);
				ss->estat_actual->moviments[ss->nummovimentlegal].puntuacio = punt;

				ml++;
				ss->nummovimentlegal++;
				ss->estat_actual->fi = &ss->estat_actual->moviments[ss->nummovimentlegal];
			}
			ss->estat_actual->moviments[ss->nummovimentlegal].moviment = 0;
			if (ss->numthread == 0)
				nummovimentsroot = ss->nummovimentlegal;
			multipv = min(MultiPV, ss->nummovimentlegal - 1);
			if (multipv == 0)
				multipv = 1;
			if (ssbase[0].MillorsMovimentsPv[pvactual] == JugadaInvalida)
				OrdenacioEstableJugades(ss->estat_actual);
		}

#ifdef RANDOM
		if (ss->numthread == 0 && mirarfinsnivell == 1) {
			//el primer cop ja es decideix quina serà la jugada bona, tot i que fa l'anàlisi complert normalment
			Bitboard jjj = mt_llrand() % 10000;
			jugadarandom = 0;
			if (jjj < Random) {
				jjj = mt_llrand() % nummovimentsroot;
				jugadarandom = ss->estat_actual->moviments[jjj].moviment;
			}
		}
#endif

		ss->nummovimentlegal = 0;
		struct thash auxhash;
		struct thash *auxhashp;

		if (mirarfinsnivell == 1 && pvactual == 0) {
			auxhashp = buscar_hash(ss->estat_actual->hash);
			if (auxhashp)
				auxhash = *auxhashp;
			else
				auxhash.w1 = 0;
			if (auxhash.w1 != 0)
				ss->MillorsMovimentsPv[pvactual] = movimenthash(auxhash);
		}
		//guardar puntuacions anteriors
		int antpuntpv[100];
		//donar puntuació màxima al millor moviment de la iteració anterior
		if (mirarfinsnivell > 1 || ssbase[0].MillorsMovimentsPv[pvactual] != JugadaInvalida) {
			for (movact = ss->estat_actual->moviments; movact < ss->estat_actual->moviments + nummovimentsroot; movact++) { //-> fi apunta a un altre ssbase si numthread <> 0
				for (int x = 0; x < multipv; x++) {
					if (movact->moviment == ssbase[0].MillorsMovimentsPv[x]) {
						antpuntpv[x] = movact->puntuacio;
						movact->puntuacio = maxpuntuacio;
						movact->puntuacio += (100 - x); //per poder saltar les pv anteriors en mode multipv
						break;
					}
				}
			}
			//NO: ordenar resta de jugades segons número de jugades que ha mirat en depth anterior
			// no cambia ordre 1r element, doncs és el millor
			OrdenacioEstableJugades(ss->estat_actual);
		}
		movact = ss->estat_actual->moviments;

		ss->mirant1ajugada = true;
		numjugadesb = ss->numjugadesfetes;
		bool reduirnodes = false;

		while (movact->moviment) {
			long long auxll = maxpuntuacio;
			auxll += (100 - pvactual);
			if (movact->puntuacio > auxll) {
				movact->puntuacio = antpuntpv[abs((long long)(movact->puntuacio - maxpuntuacio - 100))]; //recuperar puntuació que hi havia
				movact++;
				continue; //saltar pv anteriors
			}
			ss->nummovimentlegal++;
			//assegurar que sempre hi hagi un moviment a fer
			if (ss->MillorsMovimentsPv[pvactual] == JugadaInvalida)
				ss->MillorsMovimentsPv[pvactual] = movact->moviment;

			if (ss->numthread == 0 && mirarfinsnivell > 15 && (temps_per_moure == 0 || temps_per_moure > 10000)) {
				char texte[10];
				moviment_a_texte_simple(movact->moviment, texte);
				sprintf(infostring, "info depth %d currmove %s currmovenumber %d\n", mirarfinsnivell, texte, ss->nummovimentlegal);
			}

			bool jugadaesescac; jugadaesescac = es_escac_bo(ss, movact->moviment);
			fer_moviment(ss, movact->moviment);
			ss->estat_actual++;
			ss->estat_actual->escac = jugadaesescac;
			ss->estat_actual->nullmovethreat = JugadaInvalida;

			int noudepth = mirarfinsnivell;
			if (mirarfinsnivell > 4 && numcopsavaluaciosimilar < 5) {
				if (ss->nummovimentlegal > 5) {
					if (alpha - 10 > ss->puntuacio_final)
						noudepth--;
					if (ss->nummovimentlegal > 10) {
						if (alpha - 10 > ss->puntuacio_final)
							noudepth--;
						if (alpha - 30 > ss->puntuacio_final)
							noudepth--;
						if (ss->nummovimentlegal > 15) {
							if (alpha - 10 > ss->puntuacio_final)
								noudepth--;
							if (alpha - 30 > ss->puntuacio_final)
								noudepth--;
							if (alpha - 80 > ss->puntuacio_final)
								noudepth--;
							if (ss->nummovimentlegal > 20) {
								if (alpha - 10 > ss->puntuacio_final)
									noudepth--;
								if (alpha - 30 > ss->puntuacio_final)
									noudepth--;
								if (alpha - 100 > ss->puntuacio_final)
									noudepth--;
							}
						}
					}
				}
				if (ss->nummovimentlegal >= (nummovimentsroot - (nummovimentsroot >> 3)))
					noudepth--;
				if (noudepth < 2)
					noudepth = 2;
			}
			if (PunI(ss->estat_actual->valorpeces[ss->estat_actual->mou]) <= 1500 || mirarfinsnivell - noudepth > 4) {
				int u = (mirarfinsnivell - noudepth) >> 1;
				noudepth = mirarfinsnivell - u;
			}

			if (noudepth == mirarfinsnivell && mirarfinsnivell > 4 && ss->nummovimentlegal > 2 && !jugadaesescac) {
				if (ss->nummovimentlegal > 10)
					noudepth--;
				noudepth--;
			}

			if (ss->nummovimentlegal == 1) //si és el 1r moviment
				score = -alpha_beta<SI_PV>(ss, -beta, -alpha, 1, null_si, noudepth, false, false);
			else {
				//1r intentar refutar
				score = -alpha_beta<NO_PV>(ss, -alpha - 1, -alpha, 1, null_si, noudepth, true, false);
				if (ss->temps_acabat) {
					goto temps_acabat_root;
				}
				if (score > alpha)
					score = -alpha_beta<SI_PV>(ss, -beta, -alpha, 1, null_si, noudepth, false, false);
			}
			if (ss->temps_acabat) {
				goto temps_acabat_root;
			}
			long long lll; lll = ((long long)(movact->puntuacio)) + ss->numjugadesfetes - numjugadesb;
			if (lll > maxpuntuacio)
				reduirnodes = true;
			if (reduirnodes)
				movact->puntuacio = lll >> 5;
			else
				movact->puntuacio = lll;
			ss->mirant1ajugada = false;

		temps_acabat_root:
			desfer_moviment(ss, ss->estat_actual->moviment_previ);

			if (ss->temps_acabat) {
				ss->score = 0;
				return;
			}

			numjugadesb = ss->numjugadesfetes;

			if (score > alpha) {
				if (ss->numthread == 0 && pvactual == 0 && ss->nummovimentlegal != 1 && mirarfinsnivell >= 5)
					canviadajugadabona++;
				ss->MillorsMovimentsPv[pvactual] = movact->moviment;

				if (ss->seldepth < mirarfinsnivell)
					ss->seldepth = mirarfinsnivell; //així estalvia incrementar-ho dins alpha_beta;

				// actualitza PV
				ss->pv[0][0] = movact->moviment;
				memcpy(&ss->pv[0][1], &ss->pv[1][0], MaxProfunditat * sizeof(Moviment));
				ss->pv[0][MaxProfunditat] = 0;
				ss->moviment_a_ponderar = ss->pv[0][1];

				if (ss->numthread == 0 && printfpensar && (temps_per_moure == 0 || temps_per_moure > 10000 || mirarfinsnivell > 10)) {
					mostrar_pv(ss, score, alpha, beta, pvactual, mirarfinsnivell);
					ss->mostradapv = true;
				}
				ss->seldepth = 0;  //és la profunditat màxima que ha usat per analitzar la variant principal
				if (score >= beta) {
					ss->score = score;
					ss->acabatroot = true;
					ss->acabatbe = true;
					ss->pararthread0 = true;
					return;
				}
				alpha = score;
				if (pvactual == 0)
					guardar_hash(alpha, 0, mirarfinsnivell, ss->estat_actual->hash, tt_alpha, ss->MillorsMovimentsPv[pvactual], JugadaInvalida, ss->estat_actual->avaluacio);
			}
			movact++;
		};

		if (ss->nummovimentlegal == 0) { //mat o ofegat
			if (pvactual != 0)
				goto fifi;
			if (ss->estat_actual->escac) {//mat
				alpha = maxalpha;
			}
			else  //ofegat
				alpha = 0;
		}

		if (pvactual == 0)
			guardar_hash(alpha, 0, mirarfinsnivell, ss->estat_actual->hash, tt_exacte, ss->MillorsMovimentsPv[pvactual], JugadaInvalida, ss->estat_actual->avaluacio);

	fifi:
		ss->score = alpha;
		ss->acabatroot = true;
		ss->acabatbe = true;
		ss->pararthread0 = true;
		return;
	}  //search_root


	void mirar_temps() {
		seguent_mirar_temps = nodes_mirar_temps;
		if (tasca == tasca_ponderar)
			goto noacabat;
		if ((temps_tipus & TipusInfinit) || (temps_tipus & TipusProfunditat))
			goto noacabat;
		if (tasca == tasca_cap)
			ssbase[0].temps_acabat = true;
		else {
			if (temps_tipus & TipusNodes) {
				if (ssbase[0].numjugadesfetes > temps_nodes)
					ssbase[0].temps_acabat = true;
			}
			else {
				ssbase[0].temps_acabat = mirar_si_parar();
			}
		}
		temps_acabat_real = ssbase[0].temps_acabat;
	noacabat:
		if (printfpensar && infostring[0]) {
			afout("%s", infostring);
			infostring[0] = 0;
		}
		if (!ssbase[0].temps_acabat && (temps_per_moure == 0 || temps_per_moure > 500 || tasca == tasca_ponderar) && entrada_pendent())
			comandes_uci(false);
	}


	int valornullmove[] = { 0, 0, -140, -105, -70, -35, 0, 32, 65, 95, 130, 160, 195, 240, 275, maxbeta };

	template <bool es_pv>
	int alpha_beta(tss * RESTRICT ss, int alpha, int beta, int n, bool pot_null, int mirarfinsnivell, bool cutNode, bool ProvarMoviments) {
		int f_prune = 0;
		int extes = 0;
		int extensioescac = 0;
		Estat_Analisis *estat_actual = ss->estat_actual;
		bool taules = false;
		if (ss->temps_acabat || n >= MaxProfunditat) return 0;

		if (ss->numthread == 0 && --seguent_mirar_temps <= 0) {
			mirar_temps();
			if (ss->temps_acabat) return 0;
		}
		if (son_taules(ss, n)) {
			if (estat_actual->escac)
				return ValorTaules[estat_actual->mou];
			taules = true;
		}

		if (es_pv)
			ss->pv[n][0] = 0;

		//extensió per canvi de peça a línies_pv
		if (estat_actual->menjat != CasellaBuida && es_pv &&
			((ss->mirant1ajugada && mirarfinsnivell <= ss->mirarfinsnivellbase + 4) || mirarfinsnivell <= ss->mirarfinsnivellbase)) //+5
		{
			mirarfinsnivell++;
			extes++;
		}

		if (n == mirarfinsnivell) {
			if (estat_actual->escac) {
				mirarfinsnivell++;
				extes++;
			}
			else {
				int i = quiesce(ss, alpha, beta, n, true, -1);
				if (taules) {
					if (i > 0)
						return ValorTaules[estat_actual->mou] + 1;
					if (i < 0)
						return ValorTaules[estat_actual->mou] - 1;
					return ValorTaules[estat_actual->mou];
				}
				else
					return i;
			}
		}

		alpha = max(-maxbeta + n, alpha);
		beta = min(maxbeta - n, beta);
		if (alpha > beta || (alpha == beta && abs(alpha) >= maxbeta - MaxProfunditat)) {
			if (taules)
				return ValorTaules[estat_actual->mou];
			return alpha;
		}

		struct thash auxhash;
		int score, rBeta;
		int depth = mirarfinsnivell - n;
		estat_actual->reduc_lmr = 0;
		auxhash.depth = -1;

		estat_actual->movimenthash = JugadaInvalida;
		estat_actual->segonmovimenthash = JugadaInvalida;
		estat_actual->moviments[0].moviment = JugadaInvalida;
		estat_actual->avaluacio = puntuacioinvalida;

		(estat_actual + 1)->MovimentExclos = JugadaInvalida;

		struct thash *auxhashp;
		Moviment MovimentExclos = estat_actual->MovimentExclos;
		ClauHash clauhash = MovimentExclos ? (estat_actual->hash ^ ClauMovimentExclos) : estat_actual->hash;
		auxhashp = buscar_hash(clauhash);
		if (auxhashp)
			auxhash = *auxhashp;
		else
			auxhash.w1 = 0;
		if (auxhash.w1 != 0){
			if (valorhash(auxhash) != puntuacioinvalida) {
				estat_actual->avaluacio = valorhash(auxhash);
				if (taules) {
					if (estat_actual->avaluacio > 0)
						return ValorTaules[estat_actual->mou] + 1;
					if (estat_actual->avaluacio < 0)
						return ValorTaules[estat_actual->mou] - 1;
					return ValorTaules[estat_actual->mou];
				}
			}
			score = auxhash.score;
			Moviment mx = movimenthash(auxhash);
			if (!es_pv
				&& auxhash.depth >= depth
				&& (score > alpha && score < beta ? auxhash.tipuspuntuaciohash == tt_exacte
				: score >= beta ? (auxhash.tipuspuntuaciohash == tt_beta)
				: (auxhash.tipuspuntuaciohash == tt_alpha))
				) {
				if (score > maxbeta - 300)
					score -= n - 1;
				else if (score < -maxbeta + 300)
					score += n - 1;
				auxhashp->edat = edathash;

				if (score > alpha
					&& mx != JugadaInvalida
					&& ss->tau.c[Desti(mx)] == CasellaBuida
					&& !EsPromocio(mx)
					&& !EsAlPas(mx))
				{
					if (mx != estat_actual->killers[0])
						estat_actual->killers[1] = estat_actual->killers[0];
					estat_actual->killers[0] = mx;
					guardar_refutacio(obtenir_clau_refutacio(ss), mx);
					guardar_refutacio3(obtenir_clau_refutacio3(ss), mx);
					setHistory(ss, mx, false, (depth)* (depth));
				}
				return score;
			}

			if (!es_pv && ss->nummovimentlegal > 4 && depth < 5 && (estat_actual - 1)->fase == etapa_captures_dolentes) {
				ss->prunepocimportants--;
				if (!ss->prunepocimportants) {
					ss->prunepocimportants = CadaNPrunePocImportantsBase;
					return beta;
				}
			}

			if (mx != JugadaInvalida) {
				estat_actual->movimenthash = mx;
			}
			estat_actual->segonmovimenthash = auxhash.segonmoviment;
			if ((auxhash.tipuspuntuaciohash == tt_alpha) && (depth)-2 - 1 <= auxhash.depth && auxhash.score < beta)
				pot_null = false;
		}

		if (estat_actual->calculaatacs) {
			calcular_atacs(ss);
			estat_actual->calculaatacs = false;
		}

		estat_actual->clavats = marcar_clavats(ss, 1, estat_actual->mou);
		estat_actual->descoberts = marcar_clavats(ss, 0, estat_actual->mou);
		estat_actual->clavatsrival = Tota1BB;

		Moviment nounullmovethreat = JugadaInvalida;
		estat_actual->n = n;
		estat_actual->nivell = depth;

#ifdef TB
#ifdef TB2
		if (false && TB_LARGEST > 0 && popcount(TTotespeces) <= TB_LARGEST) {
#else
		if (TB_LARGEST > 0 && popcount(TTotespeces) <= TB_LARGEST) {
#endif
			unsigned res = tb_probe_wdl(Tpeces(blanques), Tpeces(negres),
				Trei(blanques) | Trei(negres),
				Tdames(blanques) | Tdames(negres),
				Ttorres(blanques) | Ttorres(negres),
				Talfils(blanques) | Talfils(negres),
				Tcaballs(blanques) | Tcaballs(negres),
				Tpeons(blanques) | Tpeons(negres),
				estat_actual->jugades50,
				estat_actual->enroc,
				estat_actual->casellapeoalpas,
				(estat_actual->mou == blanques));
			if (res != TB_RESULT_FAILED) {
				ss->tbhits++;
				guardar_hash(TbValues[res], n, TbDepth, clauhash, tt_exacte, JugadaInvalida, TbValues[res]);
				return TbValues[res];
			}
		}
#endif

		/*int32_t jjj = mt_lrand();
		estat_actual->movimenthash = (jjj & 63) * 63 + (jjj & 16128);

		int lll = mt_lrand();
		if (lll & 1)
		estat_actual->movimenthash |= flag_enroc;

		if (lll & 2)
		estat_actual->movimenthash |= flag_peo_al_pas;

		if (lll & 4)
		estat_actual->movimenthash |= flag_corona_dama;

		if (lll & 8)
		estat_actual->movimenthash |= flag_corona_torre;

		if (lll & 16)
		estat_actual->movimenthash |= flag_corona_caball;

		if (lll & 32)
		estat_actual->movimenthash |= flag_corona_alfil;

		auxhash.tipuspuntuaciohash = tt_beta;
		auxhash.depth = 3;
		*/

		if (estat_actual->escac) {
			//https://github.com/jhellis3/Stockfish/commit/13cda5f501de6dfa18dec1bdaa4362ce57e83255
			if (ss->estat[n - 1].avaluacio != puntuacioinvalida)
				estat_actual->avaluacio = -ss->estat[n - 1].avaluacio;
		}
		if (estat_actual->avaluacio == puntuacioinvalida) {
			avaluacio(ss);
			if (auxhash.w1 == 0)
				guardar_hash(scoreinvalid, n, tt_sensedepth, clauhash, tt_res, JugadaInvalida, JugadaInvalida, estat_actual->avaluacio);
		}

		if (!estat_actual->escac) {
			if (taules) {
				if (estat_actual->avaluacio > 0)
					return ValorTaules[estat_actual->mou] + 1;
				if (estat_actual->avaluacio < 0)
					return ValorTaules[estat_actual->mou] - 1;
				return ValorTaules[estat_actual->mou];
			}
			if (ProvarMoviments)
				goto ferProvarMoviments;

			if (depth > max(NivellFutilityPruning, DepthRazoring))
				goto movimentnull;

			if (depth <= 4
				&& abs(beta) < maxbeta - MaxProfunditat
				&& estat_actual->avaluacio - EvalMargin(depth) >= beta
				) {
				return estat_actual->avaluacio - EvalMargin(depth);
			}

			//http://www.talkchess.com/forum/viewtopic.php?start=0&t=50587&topic_view=flat&sid=5106c884287619ae8fcbfa4c909b4d54
			if (!es_pv
				&& pot_null
				&&  depth < NivellFutilityPruning
				&&  estat_actual->avaluacio - ValorFutilityPruning * depth >= beta
				&&  abs(beta) < maxbeta - MaxProfunditat
				&&  abs(estat_actual->avaluacio) < valor_victoria_coneguda
				&& (Tpeces(estat_actual->mou) - Tpeons(estat_actual->mou)) != Trei(estat_actual->mou)) {
				return estat_actual->avaluacio - ValorFutilityPruning * depth;
			}

			//Razoring
			if (!es_pv
				&& depth < DepthRazoring
				&& estat_actual->movimenthash == JugadaInvalida
				&& abs(beta) < maxbeta - MaxProfunditat
				)
			{
				score = estat_actual->avaluacio + ValorRazoring1;
				if (score < beta) {
					if (depth == 1) {
						rBeta = quiesce(ss, alpha, beta, n, true, -1);
						return rBeta;
					}
					score += ValorRazoring2;
					if (score < beta && depth <= 3) {
						rBeta = quiesce(ss, alpha, beta, n, true, -1);
						if (rBeta < beta) {
							return rBeta;
						}
						estat_actual->nivell = depth;
					}
				}
			}

			// Razoring. Hakkapeliitta 3.0
			if (!es_pv && depth <= DepthRazoring && estat_actual->avaluacio + RazorMargin2(depth) <= alpha)
			{
				const auto razoringAlpha = alpha - RazorMargin2(depth);
				rBeta = quiesce(ss, razoringAlpha, razoringAlpha + 1, n, true, -1);
				if (rBeta <= razoringAlpha)
					return rBeta;
				estat_actual->nivell = depth;
			}

		movimentnull:
			if (NullMove && beta - alpha <= 1 &&
				depth >= 2 &&
				pot_null &&
				abs(beta) < maxbeta - MaxProfunditat &&
				estat_actual->avaluacio > beta &&
				estat_actual->avaluacio >= beta - valornullmove[min(depth, 15)] &&
				PunI(estat_actual->valorpeces[estat_actual->mou]) > MaterialFinal)
			{
				int Reduction = (950 + 5 * depth) / 256 + (estat_actual->avaluacio - vpeosee >= beta);
				ss->movimentthreat = JugadaInvalida;
				fer_moviment_null(ss);
				ss->estat_actual++;
				ss->estat_actual->escac = false;
				if (depth - (Reduction + 1) <= 1)
					score = -quiesce(ss, -beta, -alpha, n + 1, true, -1);
				else {
					ss->estat_actual->nullmovethreat = JugadaInvalida;
					score = -alpha_beta<NO_PV>(ss, -beta, -alpha, n + 1, null_no, mirarfinsnivell - (Reduction + 1), !cutNode, true);
				}
				desfer_moviment_null(ss);
				ss->estat_actual--;
				if (score >= beta) {
					return score < maxbeta - MaxProfunditat ? score : beta;
				}
				estat_actual->nivell = depth;
				nounullmovethreat = ss->movimentthreat;
			}

			//http://chessprogramming.wikispaces.com/CPW-Engine_search
			if (!es_pv && mirarfinsnivell > 3 && depth <= 2 &&
				abs(alpha) < (maxbeta - 999) &&
				estat_actual->avaluacio + fmargin[depth] <= alpha)
				f_prune = 1;

			// IID Internal iterative deepening
			if (depth >= 8 && estat_actual->movimenthash == JugadaInvalida)
			{
				int i = depth - 2 - (es_pv ? 0 : depth / 4);
				score = -alpha_beta<es_pv>(ss, alpha, beta, n, null_no, n + i, true, true);
				auxhashp = buscar_hash(clauhash);
				if (auxhashp){
					if (movimenthash_p(auxhashp) != JugadaInvalida)
						estat_actual->movimenthash = movimenthash_p(auxhashp);
					auxhash = *auxhashp;
				}
				estat_actual->nivell = depth;
			}
		} //no escac

	ferProvarMoviments:
		int nummovimentlegal = 0, reduc_lmr = 0;
		Moviment millormoviment = JugadaInvalida;
		Moviment segonmoviment = JugadaInvalida;
		int m;
		uint8_t tt_flag = tt_alpha;
		bool jugadaesescac, fenthash = false;
		int valorsee = -9999;

		int CasellaMovimentPrevi = Desti(estat_actual->moviment_previ);
		int LMR = 0; //es posa a zero per evitar warning

		int millorant;
		if ((n >= 2 && estat_actual->avaluacio != puntuacioinvalida && ss->estat[n - 2].avaluacio != puntuacioinvalida && estat_actual->avaluacio >= ss->estat[n - 2].avaluacio))
			millorant = estat_actual->avaluacio - ss->estat[n - 2].avaluacio;
		else
			millorant = 0;

		int nummovimentsfutils;
		if (depth < MaxMovimentsFutils) {
			nummovimentsfutils = MovimentsFutils[depth];
			if (millorant) {
				int incr;
				if (millorant > 200) //200 //no: 100 //quasi igual: 300
					incr = nummovimentsfutils >> 1;
				else
					incr = nummovimentsfutils >> 2;
				nummovimentsfutils += incr;
			}
		}

		if (n < MaxProfunditat - 2 && !(estat_actual - 1)->escac) {
			//http://talkchess.com/forum/viewtopic.php?t=56540
			(estat_actual + 2)->killers[0] = JugadaInvalida;
			(estat_actual + 2)->killers[1] = JugadaInvalida;
		}

		if ((m = estat_actual->movimenthash)) {
			if (jugada_legal(ss, m, (e_colors)estat_actual->mou)) {
				if (jugada_ilegal(ss, m)) {
					goto no_fer_hashab;
				}
				fenthash = true;
				goto fer_jugada_hash;
			}
		}

	no_fer_hashab:
		estat_actual->countermoves[0] = ss->countermoves[ss->tau.c[CasellaMovimentPrevi]][CasellaMovimentPrevi].primera;
		estat_actual->countermoves[1] = ss->countermoves[ss->tau.c[CasellaMovimentPrevi]][CasellaMovimentPrevi].segona;

		LMR = 0;
		estat_actual->actual = NULL;
		estat_actual->fase = etapa_res;
		estat_actual->refutacio = obtenir_refutacio(obtenir_clau_refutacio(ss));
		estat_actual->refutacio3 = obtenir_refutacio3(obtenir_clau_refutacio3(ss));

		while ((m = moviment_ab(ss))) {
			if (jugada_ilegal(ss, m))
				continue;
			if (m == MovimentExclos)
				continue;
			if (!es_pv && n >= mirarfinsnivell - 1 && nummovimentlegal > 4)
				return alpha;
			valorsee = -9999;
			if (estat_actual->fase == etapa_bones_captures ||
				((estat_actual->fase == etapa_quiet) && depth > 3) //3  dolent: 2  igual: 4
				) {
				ss->menjariaambpeoclavat = false;
				if ((estat_actual->actual->puntuacio = valorsee = see(ss, m, 0, (e_colors)estat_actual->mou)) < 0 && !ss->menjariaambpeoclavat) {
				jugadapotserdolenta:
					if (estat_actual->seguentsegonesjugades - &estat_actual->segonesjugades[0] >= MAX_SEGONESJUGADES - 1)
						goto fer_jugada_hash; //no n'hi caben més
					estat_actual->seguentsegonesjugades->moviment = m;
					estat_actual->seguentsegonesjugades->puntuacio = estat_actual->actual->puntuacio;
					estat_actual->seguentsegonesjugades++;
					continue;
				}
			}

		fer_jugada_hash:
			nummovimentlegal++;
			if (n == 1 && ss->mirant1ajugada && nummovimentlegal > 10)
				ss->mirant1ajugada = false;
			jugadaesescac = es_escac_bo(ss, m);
			reduc_lmr = extensioescac = 0;

			if (jugadaesescac && !extes && (nummovimentlegal == 1 || !(depth < MaxMovimentsFutils && nummovimentlegal >= nummovimentsfutils))) {
				if (valorsee == -9999) {
					valorsee = see(ss, m, 0, (e_colors)estat_actual->mou);
				}
				if (valorsee >= 0) {
					extensioescac = 1;
				}
			}

			//extensió singular
			if (fenthash
				&& depth >= 8 //8 //no: 7 bo a ràpides, 9
				&& !extensioescac
				&& !MovimentExclos
				&& auxhash.tipuspuntuaciohash == tt_beta
				&& auxhash.depth >= depth - 3 //-3 //no: -2, -4
				&& abs(auxhash.score) < (maxbeta - 999))
			{
				int jj = auxhash.score - 3 * (depth / 2);
				estat_actual->MovimentExclos = m;
				int score2 = alpha_beta<NO_PV>(ss, jj - 1, jj, n, null_si, n + depth / 2 - 1, cutNode, true);
				estat_actual->MovimentExclos = JugadaInvalida;
				if (score2 < jj)
					extensioescac = 1;
				estat_actual->nivell = depth;
			}

			bool jugadaperillosa = false;

			++LMR;
			int pecamou = ss->tau.c[Origen(m)];
			if (!fenthash && ss->tau.c[Desti(m)] == CasellaBuida
				&& !EsEspecial(m)) {
				if (peca_sense_color(pecamou) == PeoBlanc) {
					jugadaperillosa = estat_actual->mou ? ROW(Desti(m)) < 3 : ROW(Desti(m)) > 4;
				}
				if (!jugadaperillosa && peca_sense_color(pecamou) == CaballBlanc
					&& estat_actual->fase != etapa_captures_dolentes
					&& Tdames(oponent(estat_actual->mou))
					&& (Tdames(oponent(estat_actual->mou)) & AtacsCaball[Desti(m)]))
					jugadaperillosa = true;

				if (depth >= nivelllmr && (!(n == 1 && nummovimentlegal < 5))
					&& !(es_pv && nummovimentlegal == 1)
					&& m != estat_actual->killers[0]
					&& m != estat_actual->killers[1]
					&& m != estat_actual->nullmovethreat
					&& m != estat_actual->refutacio
					&& m != estat_actual->refutacio3
					)
				{
					//Discocheck
					if (!estat_actual->escac && !jugadaesescac && !jugadaperillosa && depth <= 8 && ss->mirarfinsnivellbase>5 &&
						nummovimentlegal > 1 &&
						(abs(alpha) < (maxbeta - 999)) &&
						!(es_pv && PunI(estat_actual->valorpeces[estat_actual->mou]) < 1500)
						) {
						if (LMR >= 3 + depth * (2 * depth - 1) / 2) {
							continue;
						}
					}

					if (!estat_actual->escac && !jugadaperillosa && depth < MaxMovimentsFutils
						&& nummovimentlegal >= nummovimentsfutils) {
						if (depth < 6 && estat_actual->fase == etapa_quiet) {
							estat_actual->actual = NULL; //passar a etapa_captures_dolentes
						}
						continue;
					}

					int nn = depth;
					if (PunI(estat_actual->valorpeces[estat_actual->mou]) < MaterialFinal2)
						nn++; //més reducció a final
					reduc_lmr = reduccio_lmr[min(63, nn)][min(maxjugadeslmr - 1, LMR)];

					if (!es_pv && cutNode) {
						reduc_lmr += 1 + (nummovimentlegal > 3);
					}
					else {
						if (PuntuacioHistorial(pecamou, Desti(m)) < 0) {
							reduc_lmr++;
						}
					}

					if (m == estat_actual->countermoves[0] || m == estat_actual->countermoves[1]) {
						reduc_lmr--;
					}

					if (nummovimentlegal > 7 && PunI(estat_actual->valorpeces[estat_actual->mou]) < MaterialFinal2) {
						reduc_lmr++;
					}

					if (n < 7 && ss->puntuacio_final == 0 && ss->puntuacio_final_ant == 0) {
						reduc_lmr -= 1;
					}

					if (jugadaesescac) {
						reduc_lmr -= 1;
					}

					reduc_lmr -= extensioescac;

					if (reduc_lmr < 0)
						reduc_lmr = 0;

					if (reduc_lmr > 0 && n + 1 > mirarfinsnivell - reduc_lmr) {
						continue; //si es passa de lmr, prune
					}
					estat_actual->reduc_lmr = reduc_lmr;
				}

				if (f_prune
					&& nummovimentlegal > 1
					&& !jugadaesescac
					&& !jugadaperillosa
					) {
					continue;
				}

				//prune see
				if (!es_pv
					&& reduc_lmr > 0
					&& depth - reduc_lmr < 4
					&& abs(alpha) < (maxbeta - 999)
					&& !jugadaesescac
					&& !jugadaperillosa
					) {
					if (valorsee == -9999) {
						if (estat_actual->fase == etapa_captures_dolentes)
							valorsee = estat_actual->actual->puntuacio;
						else {
							valorsee = see(ss, m, 0, (e_colors)estat_actual->mou);
						}
					}
					if (valorsee < 0)
						continue;
				}
			}

			else if (
				!fenthash
				&& estat_actual->avaluacio != puntuacioinvalida
				&& depth < 3
				&& !estat_actual->escac
				&&  abs(alpha) < (maxbeta - 999)
				&& (ss->tau.c[Desti(m)] != CasellaBuida
				|| EsPromocio(m)
				|| jugadaesescac
				)
				)
			{
				if (valorsee == -9999) {
					if (estat_actual->fase == etapa_captures_dolentes)
						valorsee = estat_actual->actual->puntuacio;
					else {
						valorsee = see(ss, m, 0, (e_colors)estat_actual->mou);
					}
				}
				if (valorsee < estat_actual->avaluacio - alpha - 200)
					continue;
			}

			//reduïr una mica captures o killers
			if (!fenthash
				&& (ss->tau.c[Desti(m)] != CasellaBuida || m == estat_actual->killers[1])
				&& depth >= nivelllmr
				&& depth < MaxMovimentsFutils
				&& nummovimentlegal >= nummovimentsfutils)
				reduc_lmr = 1;


			// Intentar preveure un posterior eval pruning - parent node
			if (reduc_lmr && estat_actual->avaluacio != puntuacioinvalida && mirarfinsnivell - reduc_lmr - n - 1 <= 4 //no: RazorDepth (3), 5
				&& !jugadaesescac
				&& !jugadaperillosa
				&& abs(beta) < maxbeta - MaxProfunditat
				&& (-estat_actual->avaluacio) - EvalMargin(mirarfinsnivell - reduc_lmr - n - 1) - 50 >= -alpha
				) {
				continue;
			}

			fer_moviment(ss, m);

			bool FetaExtensioPeo7a = false;
			//extensió per peo arribar a 7a
			if (!extes && !extensioescac &&
				((ROW(Desti(m)) == 6 && ss->tau.c[Desti(m)] == PeoBlanc) || (ROW(Desti(m)) == 1 && ss->tau.c[Desti(m)] == PeoNegre)) &&
				(estat_actual->actual == NULL || estat_actual->actual->puntuacio > valorcapturesdolentes)) { //no fer-ho si té SEE negatiu
				if (mirarfinsnivell - ss->mirarfinsnivellbase < ExtensioMaxima) {
					mirarfinsnivell++;
					FetaExtensioPeo7a = true;
				}
			}

			ss->estat_actual++;
			ss->estat_actual->escac = jugadaesescac;

			bool feranalisinormal = true;

			ss->estat_actual->nullmovethreat = nounullmovethreat;

			if (reduc_lmr > 0 && !FetaExtensioPeo7a)		//** no té molt sentit que pugui haver extés i fer lmr igualment
			{
				//reduïr menys si peça escapa d'atac
				if (peca_sense_color(ss->tau.c[Desti(m)]) != PeoBlanc
					&& see(ss, FerMoviment(Desti(m), Origen(m)), 0, (e_colors)oponent(ss->estat_actual->mou)) < 0)
					reduc_lmr -= 1 + (nummovimentlegal > 3);
				if (reduc_lmr > 0) {
					score = -alpha_beta<NO_PV>(ss, -(alpha + 1), -alpha, n + 1, null_si, mirarfinsnivell - reduc_lmr, true, false);

					if (score > alpha && reduc_lmr >= 4)
					{
						score = -alpha_beta<NO_PV>(ss, -(alpha + 1), -alpha, n + 1, null_si, mirarfinsnivell - reduc_lmr + 2, true, false);
					}

					if (score <= alpha) {
						feranalisinormal = false;
					}
				}
			}
			estat_actual->reduc_lmr = 0;

			if (feranalisinormal) {
				int reduccio = depth > 1
					&& nummovimentlegal != 1 && estat_actual->fase == etapa_captures_dolentes;  //** no es dona mai nummovimentlegal==0 !!

				if (!(es_pv && nummovimentlegal == 1)) { //no es fa si és pv i és la 1a jugada
					score = -alpha_beta<NO_PV>(ss, -(alpha + 1), -alpha, n + 1, null_si, mirarfinsnivell - reduccio + extensioescac, !cutNode, false);
				}
				if (es_pv && ((es_pv && nummovimentlegal == 1) || (score > alpha && score < beta))) {
					score = -alpha_beta<SI_PV>(ss, -beta, -alpha, n + 1, null_si, mirarfinsnivell - reduccio + extensioescac, false, false);
				}
			}

			if (FetaExtensioPeo7a) {
				FetaExtensioPeo7a = false;
				mirarfinsnivell--;
			}
			desfer_moviment(ss, m);

			if (ss->temps_acabat) return 0;

			if (score > alpha) {
				if (millormoviment != JugadaInvalida) {
					segonmoviment = millormoviment;
				}
				millormoviment = m;
				if (score >= beta) {
					if (!MovimentExclos) {
#ifdef WINDOWS
						_mm_prefetch((char*)primera_entrada(clauhash), _MM_HINT_T0);
#endif
#ifdef LINUX
						__builtin_prefetch(primera_entrada(clauhash));
#endif
					}
					ss->movimentthreat = m;

					if (ss->tau.c[Desti(m)] == CasellaBuida
						&& !EsPromocio(m)
						&& !EsAlPas(m)
						) {
						if (m != estat_actual->killers[0])
							estat_actual->killers[1] = estat_actual->killers[0];
						estat_actual->killers[0] = m;
						guardar_refutacio(obtenir_clau_refutacio(ss), m);
						guardar_refutacio3(obtenir_clau_refutacio3(ss), m);
						setHistory(ss, m, !fenthash, (depth)* (depth));

						if (estat_actual->moviment_previ) {
							actualitza_countermoves(ss, ss->tau.c[CasellaMovimentPrevi], CasellaMovimentPrevi, m);
						}
					}

					tt_flag = tt_beta;
					alpha = beta;
					break;
				}
				tt_flag = tt_exacte;
				alpha = score;
				//pv
				ss->pv[n][0] = m;
				memcpy(&ss->pv[n][1], &ss->pv[n + 1][0], MaxProfunditat * sizeof(Moviment));
				ss->pv[n][MaxProfunditat] = 0;
			}
			if (fenthash) {
				fenthash = false;
				goto no_fer_hashab;
			}

		}

		if (nummovimentlegal == 0 && !MovimentExclos) { //mat o ofegat
			if (estat_actual->escac) { //mat
				return maxalpha + n - 1;
			}
			else {
				alpha = ValorTaules[estat_actual->mou];
				tt_flag = tt_exacte;
			}
		}

		if (tt_flag == tt_beta)
			segonmoviment = JugadaInvalida;

		if (!MovimentExclos)
			guardar_hash(alpha, n, depth, clauhash, tt_flag, millormoviment, segonmoviment, estat_actual->avaluacio);

		if (millormoviment) {
			guardar_hashpeces(clauhash ^ estat_actual->hashpeons, millormoviment);
		}

		return alpha;
		}

	int quiesce(tss * RESTRICT ss, int alpha, int beta, int n, int mirarescacs, int nivell) {
		Estat_Analisis *estat_actual = ss->estat_actual;
		if (nivell != -1) {
			if (ss->numthread == 0 && --seguent_mirar_temps <= 0) {
				mirar_temps();
			}
			if (son_taules(ss, n)) {
				return ValorTaules[estat_actual->mou];
			}
		}

		if (ss->temps_acabat || n >= MaxProfunditat)
			return 0;
		if (alpha + 1 != beta)
			ss->pv[n][0] = 0;
		if (n > ss->seldepth)
			ss->seldepth = n;

		const int jo = estat_actual->mou;

		struct thash auxhash;
		int score;
		estat_actual->movimenthash = JugadaInvalida;
		estat_actual->avaluacio = puntuacioinvalida;

		struct thash *auxhashp;
		auxhashp = buscar_hash(estat_actual->hash);
		if (auxhashp)
			auxhash = *auxhashp;
		else
			auxhash.w1 = 0;
		if (auxhash.w1 != 0){
			if (auxhash.depth >= nivell) {
				score = auxhash.score;
				if (score > maxbeta - 300)
					score -= n - 1;
				else if (score < -maxbeta + 300)
					score += n - 1;
				if (auxhash.tipuspuntuaciohash == tt_exacte && score > alpha && score < beta)  {
					auxhashp->edat = edathash;
					return score;
				}

				if (auxhash.tipuspuntuaciohash == tt_beta && score >= beta)  {
					auxhashp->edat = edathash;
					return score;
				}
				if (auxhash.tipuspuntuaciohash == tt_alpha && score <= alpha) {
					auxhashp->edat = edathash;
					return score;
				}
			}

			if (movimenthash(auxhash) != JugadaInvalida) {
				estat_actual->movimenthash = movimenthash(auxhash);
			}
			if (valorhash(auxhash) != puntuacioinvalida) {
				estat_actual->avaluacio = valorhash(auxhash);
			}
		}

		bool calculat = false;
		int standpat;
		if (!estat_actual->escac) {
			if (estat_actual->avaluacio == puntuacioinvalida){

				if (estat_actual->calculaatacs) {
					calcular_atacs(ss);
					estat_actual->calculaatacs = false;
				}
				estat_actual->clavats = marcar_clavats(ss, 1, jo);
				estat_actual->descoberts = marcar_clavats(ss, 0, jo);
				estat_actual->clavatsrival = Tota1BB;
				calculat = true;
				avaluacio(ss);
				guardar_hash(scoreinvalid, n, tt_sensedepth, estat_actual->hash, tt_res, JugadaInvalida, JugadaInvalida, estat_actual->avaluacio);
			}
			standpat = estat_actual->avaluacio;

			if (standpat >= beta)
				return standpat;

			if ((alpha != beta - 1) && alpha < standpat) //només si pv_node
				alpha = standpat;

		}
		else {
			standpat = maxalpha;
			estat_actual->clavatsrival = 0; //que afecti a quiesce, només s'usa per avaluació
		}
		if (!calculat) {
			if (estat_actual->calculaatacs) {
				calcular_atacs(ss);
				estat_actual->calculaatacs = false;
			}
			estat_actual->clavats = marcar_clavats(ss, 1, jo);
			estat_actual->descoberts = marcar_clavats(ss, 0, jo);
			estat_actual->clavatsrival = Tota1BB;
		}
		int nummovimentlegal = 0, provades = 0;
		int64_t millormoviment = JugadaInvalida;
		int m;
		uint8_t tt_flag = tt_alpha;

		if ((m = estat_actual->movimenthash)) {
			if (jugada_legal(ss, m, (e_colors)jo)) {
				if (jugada_ilegal(ss, m))
					goto no_fer_hash;
				nummovimentlegal++;
				provades++;
				bool jugadaesescac; jugadaesescac = es_escac_bo(ss, m);
				fer_moviment(ss, m);
				ss->estat_actual++;
				ss->estat_actual->escac = jugadaesescac;
				score = -quiesce(ss, -beta, -alpha, n + 1, false, nivell - 1);
				desfer_moviment(ss, ss->estat_actual->moviment_previ);
				if (score > alpha) {
					if (score >= beta)
						goto cut;
					alpha = score;
					millormoviment = m;
					tt_flag = tt_exacte;
				}
			}
		}

	no_fer_hash:

		estat_actual->n = n;
		estat_actual->nivell = nivell;

		if (estat_actual->escac) {
			ss->mov = estat_actual->moviments;
			if (jo == blanques)
				generar_evasions<blanques>(ss);
			else
				generar_evasions<negres>(ss);
			marcar_captures_bones_q(ss);
			estat_actual->actual = &estat_actual->moviments[0];
			while ((m = seguent_moviment(estat_actual))) {
				if (jugada_ilegal(ss, m))
					continue;
				nummovimentlegal++;
				if (provades > 0
					&& !EsPromocio(m)
					&& abs(beta) < maxbeta - MaxProfunditat
					&& ss->tau.c[Desti(m)] == 0
					&& see(ss, m, -50, (e_colors)jo) < 0
					)
					continue;
				provades++;
				bool jugadaesescac; jugadaesescac = es_escac_bo(ss, m);
				fer_moviment(ss, m);
				ss->estat_actual++;
				ss->estat_actual->escac = jugadaesescac;
				score = -quiesce(ss, -beta, -alpha, n + 1, false, nivell - 1);
				desfer_moviment(ss, ss->estat_actual->moviment_previ);
				if (ss->temps_acabat)
					return 0;
				if (score > alpha) {
					if (score >= beta) {
						goto cut;
					}
					alpha = score;
					millormoviment = m;
					tt_flag = tt_exacte;
				}
			} //per cada moviment
			goto finalq;
		}

		//captures i coronacions
		ss->mov = &estat_actual->moviments[0];
		generar_cc(ss);
		estat_actual->fi = ss->mov;  //per continuar generant moviments a partir d'aquí si s'escau
		marcar_captures_bones_q(ss);
		estat_actual->actual = &estat_actual->moviments[0];

		while ((m = seguent_moviment(estat_actual))) {
			if (jugada_ilegal(ss, m))
				continue;
			nummovimentlegal++;
			uint8_t menjat; menjat = ss->tau.c[Desti(m)];
			bool jugadaesescac; jugadaesescac = es_escac_bo(ss, m);
			//futility pruning
			if (menjat && !jugadaesescac && alpha - (standpat + valorpecasee[menjat]) > ValorDeltaCutoff) {
				if (PunI(estat_actual->valorpeces[oponent(jo)]) - valorpecasee[menjat] > MaterialFinal) {
					continue;
				}
			}
			if (see(ss, m, -50, (e_colors)jo) < 0) {
				continue;
			}
			provades++;
			fer_moviment(ss, m);
			ss->estat_actual++;
			ss->estat_actual->escac = jugadaesescac;
			score = -quiesce(ss, -beta, -alpha, n + 1, false, nivell - 1);
			desfer_moviment(ss, ss->estat_actual->moviment_previ);
			if (ss->temps_acabat)
				return 0;
			if (score > alpha) {
				if (score >= beta) {
				cut:
					ss->movimentthreat = m;
					guardar_hash(score, n, nivell, estat_actual->hash, tt_beta, m, JugadaInvalida, estat_actual->avaluacio);
					return score;
				}
				alpha = score;
				millormoviment = m;
				tt_flag = tt_exacte;
			}
		} //per cada moviment

		if (!mirarescacs)
			goto finalq;

		//etapa generar escacs
		ss->mov = estat_actual->fi;
		if (jo == blanques)
			generar_possibles_escacs(ss);
		else
			generar_possibles_escacs(ss);
		ss->mov->moviment = JugadaInvalida;
		estat_actual->actual = estat_actual->fi;

		for (;;) {
			m = estat_actual->actual->moviment;
			if (!m)
				break;
			estat_actual->actual++;
			if (m == estat_actual->movimenthash || jugada_ilegal(ss, m))
				continue;
			nummovimentlegal++;
			if (!es_escac_bo(ss, m))
				continue;
			if (see(ss, m, -50, (e_colors)jo) < 0)
				continue;
			provades++;
			fer_moviment(ss, m);
			ss->estat_actual++;
			ss->estat_actual->escac = true;
			score = -quiesce(ss, -beta, -alpha, n + 1, false, nivell - 1);
			desfer_moviment(ss, ss->estat_actual->moviment_previ);
			if (ss->temps_acabat)
				return 0;
			if (score > alpha) {
				if (score >= beta) {
					goto cut;
				}
				alpha = score;
				millormoviment = m;
				tt_flag = tt_exacte;
			}
		} //per cada moviment

	finalq:
		if (nummovimentlegal == 0) { //mat o ofegat
			if (estat_actual->escac) {//mat
				return maxalpha + n - 1; //noguardarhashmat
			}
			else  //ofegat 
				alpha = ValorTaules[jo];
		}
		if (provades == 0)
			alpha = estat_actual->avaluacio;
		if (abs(alpha) >= maxbeta - MaxProfunditat && tt_flag != tt_exacte) {
			alpha = estat_actual->avaluacio;  //per no tornar com si fos un mate quan no ha arribat a analitzar cap jugada
		}

		guardar_hash(alpha, n, nivell, estat_actual->hash, tt_flag, millormoviment, JugadaInvalida, estat_actual->avaluacio);

		return alpha;
	}

	int son_taules(tss * RESTRICT ss, int n) {
		int j;
		Estat_Analisis *pm;

		//taules per 50 moviments i per repetició
		j = ss->estat_actual->jugades50;

		if (j < 4)
			return false;

		if (j >= 100)  //**petit bug que pot ser mate a la 100, i pensaria que son taules
			return true;

		if (j > ss->estat_actual->PliesDesdeNull)
			j = ss->estat_actual->PliesDesdeNull;

		int i;
		for (i = n - 4, j = j - 4, pm = &ss->estat[i]; i >= 0 && j >= 0; i = i - 2, j = j - 2, pm -= 2) {
			if (pm->hash == ss->estat_actual->hash)
				return true;
		}
		if (j < 0)  //**potser no fa falta aquest control doncs es torna a controlar tot seguit
			return false;
		for (i = ss->darrer_ply_posicio_inicial + i; i >= ss->darrer_ply_historic_3_repeticions && j >= 0; i = i - 2, j = j - 2)
			if (ss->historic_posicions[i] == ss->estat_actual->hash)
				return true;

		return false;
	}

	int see(tss * RESTRICT ss, int moviment, int marge, e_colors jo) {
		int from, to, valorpecamou, valorpecacaptura, diferencia;
		from = Origen(moviment);
		to = Desti(moviment);
		valorpecamou = valorpecaseeambRei[ss->tau.c[from]];
		valorpecacaptura = valorpecaseeambRei[ss->tau.c[to]];
		diferencia = valorpecamou - valorpecacaptura;
		if (diferencia <= -marge)
			return 1;
		if (valorpecamou == vreisee)
			return valorpecacaptura;
		if (ss->estat_actual->descoberts & Bit(from))
			return 1; //és escac descobert. Analitzar
		if ((ss->estat_actual->clavats & Bit(from)) && valorpecamou <= valfilsee)
			return 1;  //mou una peça clavada. Analitzar. //** mirar si és bo return 1
		if (EsAlPas(moviment)) return 1;
		e_colors opo = (jo == blanques ? negres : blanques);
		Bitboard att = AtacsPeo[jo][to] & Tpeons(opo); //rival pot capturar amb peo
		if ((att) && diferencia + marge > vpeosee) {
			if (ss->estat_actual->nivell >= 0 && ss->estat_actual->clavatsrival == Tota1BB)
				ss->estat_actual->clavatsrival = marcar_clavats(ss, 1, oponent(ss->estat_actual->mou));
			if (ss->estat_actual->clavatsrival != Tota1BB && (ss->estat_actual->clavatsrival & att))
				ss->menjariaambpeoclavat = true; //per segons com mirar jugada igualment
			return valorpecacaptura - valorpecamou; //0;
		}
		if (diferencia > valfilsee - marge) {
			att |= AtacsCaball[to] & Tcaballs(opo);
			if (att)
				return valorpecacaptura - valorpecamou; //0;
		}
		Bitboard occ = TTotespeces & (~Bit(from));

		int llista[40], index = 0;
		llista[0] = valorpecacaptura;

		Bitboard totsatacants = atacants(ss, to, occ) & occ;

		Bitboard atacantsdequimou = totsatacants & Tpeces(opo);
		if (!atacantsdequimou)
			return (llista[0] > 0); //millor que retornar valorpecacaptura

		uint8_t capturat = ss->tau.c[from];

		do {
			index++;
			llista[index] = valorpecaseeambRei[capturat] - llista[index - 1];

			//trobar i treure la peça que captura de menys valor
			capturat = min_atacant(ss, opo, to, atacantsdequimou, occ, totsatacants);
			opo = (e_colors)oponent(opo);
			atacantsdequimou = totsatacants & Tpeces(opo);

			if (capturat == Rei && atacantsdequimou)
			{
				llista[++index] = vdamasee * 16; //assegurar que no agafa aquest valor mai, que si per exemple es posa vdamasee*2 podria ser més petit que el valor real
				break;
			}

		} while (atacantsdequimou);

		//negamax
		do {
			if (-llista[index] < llista[index - 1])
				llista[index - 1] = -llista[index];
		} while (--index);

		return llista[0];
	}

