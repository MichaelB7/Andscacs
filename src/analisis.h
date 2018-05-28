#ifndef INCLOS_ANALISIS_H
#define INCLOS_ANALISIS_H

#include "definicions.h"
#include <stdint.h>
#include "moviments.h"
#include "thread.h"
#include "debug.h"
#ifdef LINUX
#include <pthread.h>
#endif

struct Taulell {
	uint8_t c[64];		//taulell
	Bitboard peces[16];	//on hi ha cada tipus de peça
};

struct hp_estructura {
	uint64_t clau;
	Moviment move1, move2;
};
hp_estructura * obtenir_hashpeces(ClauHash clau);

//countermoves
struct c_estructura {
	Moviment primera, segona;
};

#define NumPeons(jo) estat_actual->numpeces[PeoBlanc + (jo)]

#define MaxPecesTipus 8

struct Estat_Analisis { 
	MovList moviments[MAX_MOVIMENTS];
	MovList segonesjugades[MAX_SEGONESJUGADES];
	ClauHash hash, hashpeons;
	Bitboard atacs[2], //de totes les peces del mateix color
		atacsalfil[2][MaxPecesTipus], atacstorre[2][MaxPecesTipus], atacsdama[2][MaxPecesTipus],  //per cada peça
		atacspeons[2], atacscaballs[2], atacsalfils[2], atacstorres[2], atacsdames[2], // de totes les peces del mateix tipus
		clavats, descoberts, clavatsrival, destinsescac, tampocpotanarrei, atacpeoacabademoure;
	int valorpeces[2];
	uint8_t numpeces[ReiNegre + 1];
	MovList *actual, *seguentsegonesjugades, *fi;
	uint16_t jugades50, PliesDesdeNull, n;
	int16_t nivell;
	Valor avaluacio;  //resultat funcio eval
	int moviment_previ, movimenthash, segonmovimenthash, nullmovethreat, refutacio, refutacio3, countermoves[2], hashpeces1, hashpeces2;
	int killers[2], MovimentExclos;
	int enroc, mou, casellapeoalpas, menjat, escac, fase, generatnomescaptures, reduc_lmr;
	bool calculaatacs;
};

struct tss {
	volatile bool temps_acabat, acabatroot, pararthread0;
	volatile long long numjugadesfetes;
	long long separador; //perquè variables prèvies no comparteixin linia cache amb les següents
	long int numthread;
	int seldepth, nummovimentlegal, prunepocimportants;
	Moviment moviment_a_ponderar;
#ifdef TB
	long long tbhits;
#endif
	bool mirant1ajugada, mostradapv;
	int puntuacio_final_ant, puntuacio_final_ant2, puntuacio_final;
	int mirarfinsnivellbase;
	Taulell tau;
	Estat_Analisis estat[MaxProfunditat];
	Estat_Analisis *estat_actual;
	int movimentthreat;
	Moviment pv[MaxProfunditat + 1][MaxProfunditat + 1];
	int16_t Historial[(ReiNegre + 1) * 64];
	c_estructura countermoves[ReiNegre + 1][64];
	bool menjariaambpeoclavat;
	ClauHash historic_posicions[MaxPly];
	int darrer_ply_historic, darrer_ply_historic_3_repeticions, darrer_ply_posicio_inicial;
	MovList *mov; //auxiliar generació moviments
	Moviment MillorsMovimentsPv[100];
	int score;
	bool acabatbe; //si pot agafar jugada de thread
};

extern tss ssbase[MaxThreads + 1]; //el darrer és per sincronitzar threads

struct parametresroot {
	volatile int alpha;
	volatile int beta;
	volatile bool iniciar, parar, analitzant, nointentarcomencar;
	long int numthread[MaxThreads];
	volatile int pvactual;
#ifdef LINUX
	pthread_t vthread[MaxThreads];
#endif
};

extern parametresroot paramroot;
extern int puntuacio_final_root;

#define TTotespeces (ss->tau.peces[blanques] | ss->tau.peces[negres])
#define Tpeces(i) ss->tau.peces[i]
#define Tpeons(jo) (Tpeces(PeoBlanc | (jo)))
#define Tcaballs(jo) (Tpeces(CaballBlanc | (jo)))
#define Talfils(jo) (Tpeces(AlfilBlanc | (jo)))
#define Ttorres(jo) (Tpeces(TorreBlanca | (jo)))
#define Tdames(jo) (Tpeces(DamaBlanca | (jo)))
#define Ttorresdames(jo) (Ttorres(jo) | Tdames(jo))
#define Trei(jo) (Tpeces(ReiBlanc | (jo)))
#define Escac(jo) (ss->estat_actual->atacs[oponent(jo)] & Trei(jo))

struct Parametres_Alpha_beta {
	uint8_t mirar_fins_nivell;
};
extern int nivellactual;
extern int seldepth;
extern Moviment darrer_moviment;

extern int color_que_juga_pc;  //es necessita pel contemp

#define PuntuacioHistorial(peca,desti) ss->Historial[((peca) << 6) | (desti)]
#define SumaHistorial(peca,desti,valor) ss->Historial[((peca) << 6) | (desti)] += (valor)
#define MaxPuntHistorial 500


extern char infostring[1024];
extern bool printfpensar, posathistoricobertures;
extern bool temps_acabat_real;
extern int64_t seguent_mirar_temps;

void neteja_varis(tss * RESTRICT ss);
void buidar_refutacio();
void buidar_hashpeces();
void buidar_refutacio3();
void buidar_countermoves(tss * RESTRICT ss);
void analitzar();
#ifdef WINDOWS
void proces_thread(void *param);
#endif
#ifdef LINUX
void* proces_thread(void *param);
#endif
void search_root(long int numthread, int pvactual);
template <bool es_pv>
int alpha_beta(tss * RESTRICT ss, int alpha, int beta, int n, bool pot_null, int mirarfinsnivell, bool cutNode, bool ProvarMoviments);
int quiesce(tss * RESTRICT ss, int alpha, int beta, int n, int mirarescacs, int nivell);
int son_taules(tss * RESTRICT ss, int n);
int see(tss * RESTRICT ss, int moviment, int margin, e_colors jo);

#define maxalpha -9999
#define maxbeta 9999
#define valor_victoria_coneguda 5000 //** revisar
#define valor_victoria_coneguda_base_avaluacio 6000 // deixa marge fins a valor_victoria_coneguda per puntuacions que s'allunyen però que igualment son victòria segura
#define maxpuntuacio (long long)(1<<30)  //per quan ordena les jugades per número de jugades analitzades, poder posar la millor com la primera
#define puntuacioinvalida -20000 // per inicialitzar tposicio.avaluacio
#define scoreinvalid 32000 //per hash

#define null_si 1
#define null_no 0

#define NO_PV 0
#define SI_PV 1

#define MaterialFinal 800 //1300  //material que es compara per només un dels contrincants
#define MaterialFinal2 2000  //per algun altre avaluació
#define NivellRecaptures -5 // a partir del nivell de recaptures, només mirar recaptures a quiesce  //**provar de canviar-lo
extern bool NullMove;
extern int MultiPV;
#ifdef RANDOM
extern int Random;
#endif
extern int Contempt;
extern bool AlwaysFullPv;
extern char tb_path[1024];

#define vpeo 85
#define vcaball 332
#define valfil 337
#define vtorre 477
#define vdama 977

#define vpeosee 90
#define vcaballsee 335
#define valfilsee 335
#define vtorresee 477
#define vdamasee 975
#define vreisee 10000

#ifdef TB
//#define TBMateValue 30000
#define TBCursedMateValue 3
const int TbValues[5] = { -(maxbeta - MaxProfunditat - 1), -TBCursedMateValue, 0, TBCursedMateValue, maxbeta - MaxProfunditat - 1 };
#define TbDepth (depth + 6 >= 127 ? 127 : depth + 6)
extern char tb_path[1024];
#endif

#endif

