#include "debug.h"

#ifdef WINDOWS
#ifdef _WIN32
#if _WIN32_WINNT < 0x0601
#undef  _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <windows.h>
#endif
#endif

#include <assert.h>
#include <string.h>
#include <math.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#ifdef WINDOWS
#include "resource.h"
#endif
#include "utils.h"
#include "analisis.h"
#include "hash.h"
#include "definicions.h"
#include "avaluacio.h"
#include "temps.h"
#ifdef LINUX
#include <stdarg.h>

//necessari per linux
#undef min
#undef max
#endif

#include "magics.h"

using namespace std;

std::string fenposini = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
std::string fenposmigjoc = "rq2kb1r/1b1n1p2/4pp2/p6p/PppPN2Q/2P2NP1/1P3PBP/R4RK1 b kq - 0 16";
std::string fenposmigjoc2 = "1r1qk2r/pp2ppbp/2np2p1/2p5/P3PPb1/3P1NP1/1PP3BP/R1BQK2R w KQk - 0 12";
std::string fenposmigjoc3 = "5rk1/2p1q1b1/1pn1p2p/1b1p1pp1/1P1Pn3/2PNBNP1/1Q2PPBP/R5K1 w - - 0 20";
std::string fenposfinal = "6R1/2kb4/8/4pp2/7r/4NP2/5K2/8 b - - 0 70";
std::string fenposfinal2 = "8/pp3p1k/6pp/7P/6P1/3K4/PPP5/8 w - - 0 38";

void afout(const char *fmt, ...)
{
	va_list args;
	char buffer[4096];

	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);

	fprintf(stdout, "%s", buffer);
	fflush(stdout);
}

void omplir_bb_peces(Taulell *taux) {
	int i;
	for (i = 0; i < 64; i++)
		if (taux->c[i] != CasellaBuida)  {
			taux->peces[taux->c[i]] |= Bit(i);
			taux->peces[taux->c[i] & 1] |= Bit(i);  //posar la peça a bb blanques o negres
		}
}

void omplir_totalpecescolor(tss * RESTRICT ss){
	int i;
	memset(&ss->estat_actual->numpeces, 0, sizeof(ss->estat_actual->numpeces));
	for (i = 0; i < 64; i++)
		if (ss->tau.c[i] != CasellaBuida)  {
			ss->estat_actual->numpeces[ss->tau.c[i]]++;
		}
}

//inicialitzar un taulell a partir d'un FEN
//exemple http://www.fam-petzke.de/cp_fen_en.shtml
//http://chessprogramming.wikispaces.com/Forsyth-Edwards+Notation
bool inicialitzar_taulell_fen(tss * RESTRICT ss, char aFEN[]){
	int i, j;
	char lletra;
	int Fila, Columna;
	char * strList;
	char * next_token = NULL;
	ss->estat_actual = &ss->estat[0];
	next_token = aFEN;
	strList = aFEN;
	while (*next_token != ' ' && *next_token != 0)
		next_token++;
	if (*next_token != ' ')
		return false;
	*next_token = 0;
	next_token++;
	//buidar tot
	memset(&ss->tau, 0, sizeof(Taulell));
	memset(ss->estat_actual, 0, sizeof(Estat_Analisis));

	int sq = cA8;
	i = 0;
	int longi = strlen(strList);
	while (i < longi) {
		lletra = strList[i];
		i++;

		if (lletra == '/') {
			sq -= 16; //fila anterior
		}
		else
			if (isdigit(lletra))
				sq += lletra - 48; //núm. de caselles buides
			else {
				switch (lletra) {
				case 'P': ss->tau.c[sq] = PeoBlanc;
					break;
				case 'R': ss->tau.c[sq] = TorreBlanca;
					break;
				case 'N': ss->tau.c[sq] = CaballBlanc;
					break;
				case 'B': ss->tau.c[sq] = AlfilBlanc;
					break;
				case 'Q': ss->tau.c[sq] = DamaBlanca;
					break;
				case 'K': ss->tau.c[sq] = ReiBlanc;
					break;
				case 'p': ss->tau.c[sq] = PeoNegre;
					break;
				case 'r': ss->tau.c[sq] = TorreNegre;
					break;
				case 'n': ss->tau.c[sq] = CaballNegre;
					break;
				case 'b': ss->tau.c[sq] = AlfilNegre;
					break;
				case 'q': ss->tau.c[sq] = DamaNegre;
					break;
				case 'k': ss->tau.c[sq] = ReiNegre;
					break;
				default:
					return false;
				}
				sq++;
			}
	}

	strList = next_token;
	while (*next_token != ' ' && *next_token != 0)
		next_token++;
	if (*next_token != ' ')
		return false;
	*next_token = 0;
	next_token++;
	ss->estat_actual->mou = blanques;
	if (strcmp(strList, "w") == 0) ss->estat_actual->mou = blanques;
	else {
		if (strcmp(strList, "b") == 0) ss->estat_actual->mou = negres;
		else
			return false;
	}

	strList = next_token;
	while (*next_token != ' ' && *next_token != 0)
		next_token++;
	if (*next_token != ' ')
		return false;
	*next_token = 0;
	next_token++;
	if (strchr(strList, 'K') != NULL)
		ss->estat_actual->enroc |= PotEnrocarBlancCurt;
	if (strchr(strList, 'Q') != NULL)
		ss->estat_actual->enroc |= PotEnrocarBlancLlarg;
	if (strchr(strList, 'k') != NULL)
		ss->estat_actual->enroc |= PotEnrocarNegreCurt;
	if (strchr(strList, 'q') != NULL)
		ss->estat_actual->enroc |= PotEnrocarNegreLlarg;

	strList = next_token;
	while (*next_token != ' ' && *next_token != 0)
		next_token++;
	if (strcmp(next_token, " moves") == 0)
		strcpy(next_token, " 0 1");
	if (*next_token == 0)
		strcpy(next_token, " 0 1");
	if (*next_token != ' ')
		return false;
	*next_token = 0;
	next_token++;
	if ((strlen(strList) >= 2)) {
		if ((strList[0] >= 'a') && (strList[0] <= 'h') && ((strList[1] == '3') || (strList[1] == '6'))) {
			Columna = strList[0] - 96; // 'a' = 97 
			Fila = strList[1] - 48; // '1' = 49
			ss->estat_actual->casellapeoalpas = ((Fila - 1) * 8 + Columna - 1);
		}
		else
			return false;
	}

	strList = next_token;
	if (*next_token == 0)
		strcpy(next_token, "0 1");
	while (*next_token != ' ' && *next_token != 0)
		next_token++;
	if (*next_token != ' ')
		return false;
	*next_token = 0;
	next_token++;
	if (strList == NULL)
		ss->estat_actual->jugades50 = 0;
	else
		ss->estat_actual->jugades50 = strtol(strList, NULL, 10);
	ss->estat_actual->PliesDesdeNull = 0;
	omplir_totalpecescolor(ss);
	posar_hash_posicio_i_valor_peces(ss);
	omplir_bb_peces(&ss->tau);
	ss->darrer_ply_historic = 0;
	ss->darrer_ply_historic_3_repeticions = 0;
	ss->darrer_ply_posicio_inicial = 0;
	ss->historic_posicions[ss->darrer_ply_historic] = ss->estat_actual->hash;
	calcular_atacs(ss);
	ss->estat_actual->clavats = marcar_clavats(ss, 1, ss->estat_actual->mou);
	ss->estat_actual->descoberts = marcar_clavats(ss, 0, ss->estat_actual->mou);
	ss->estat_actual->clavatsrival = marcar_clavats(ss, 1, oponent(ss->estat_actual->mou));
	ss->estat_actual->avaluacio = puntuacioinvalida;
	ss->estat_actual->nullmovethreat = JugadaInvalida;
	ss->estat_actual->moviment_previ = JugadaInvalida;
	ss->estat_actual->movimenthash = JugadaInvalida;
	if (ss->estat_actual->atacs[oponent(ss->estat_actual->mou)] & Trei(ss->estat_actual->mou))
		ss->estat_actual->escac = true;
	return true;
}

int convertir_a_casella(char * a) {
	int sq = 0;
	sq = a[0] - 'a';
	sq += (a[1] - '1') * 8;
	return sq;
}

int moviments_en_algebraic(tss * RESTRICT ss, char * a) {
	Estat_Analisis *est = ss->estat_actual;
	Moviment m;

	while ((a[0] >= 'a') && (a[0] <= 'h')) {

		int origen = convertir_a_casella(a);
		int desti = convertir_a_casella(a + 2);
		m = FerMoviment(origen, desti);
		uint8_t pecamou = ss->tau.c[origen];

		if (peca_sense_color(pecamou) == Peo && desti && desti == est->casellapeoalpas) //&& desti => que no sigui casella 0
			m |= flag_peo_al_pas;

		uint8_t menjat = ss->tau.c[desti];

		switch (a[4]) {
		case 'q': m |= flag_corona_dama;
			a++;
			break;
		case 'r': m |= flag_corona_torre; a++; break;
		case 'b': m |= flag_corona_alfil; a++; break;
		case 'n': m |= flag_corona_caball; a++; break;
		}

		if (peca_sense_color(pecamou) == Rei &&
			((origen == 4 && desti == 6) ||
			(origen == 60 && desti == 62))) {
			m |= flag_enroc;
			ss->darrer_ply_historic_3_repeticions = ss->darrer_ply_historic; //més endarrera no es pot repetir la posició 3 cops
		}

		if (peca_sense_color(pecamou) == Rei &&
			((origen == 4 && desti == 2) ||
			(origen == 60 && desti == 58))) {
			m |= flag_enroc;
			ss->darrer_ply_historic_3_repeticions = ss->darrer_ply_historic; //més endarrera no es pot repetir la posició 3 cops
		}

		if (menjat != CasellaBuida || peca_sense_color(pecamou) == Peo)
			ss->darrer_ply_historic_3_repeticions = ss->darrer_ply_historic; //més endarrera no es pot repetir la posició 3 cops

		ss->estat_actual->descoberts = marcar_clavats(ss, 0, ss->estat_actual->mou);
		bool jugadaesescac; jugadaesescac = es_escac_bo(ss, m);
		fer_moviment(ss, m);
		darrer_moviment = m; //per pensar una mica més si no han fet la jugada que esperava
		ss->darrer_ply_posicio_inicial = ss->darrer_ply_historic;

		//guardar només darreres 120 jugades
		if (ss->darrer_ply_historic > 120) {
			ss->darrer_ply_historic--;
			ss->darrer_ply_posicio_inicial--;
			if (ss->darrer_ply_historic_3_repeticions > 0)
				ss->darrer_ply_historic_3_repeticions--;
			//ss->historic_posicions[ss->darrer_ply_historic]
			memcpy(&ss->historic_posicions[0], &ss->historic_posicions[1], 121 * sizeof(ClauHash));
		}

		ss->estat[0] = ss->estat[1]; //la posició inicial és la darrera posició després de totes les jugades
		ss->estat_actual = &ss->estat[0];
		ss->estat_actual->escac = jugadaesescac;

		a += 4;
		while (a[0] == ' ') a++;
	}
	return false;
}

uint8_t FlagsEnroc[64];

void precalcul_flags_enroc() {
	uint8_t *a = &FlagsEnroc[0];
	memset(a, 0xFF, sizeof(FlagsEnroc));
	FlagsEnroc[0] ^= PotEnrocarBlancLlarg;
	FlagsEnroc[4] ^= (PotEnrocarBlancLlarg | PotEnrocarBlancCurt);
	FlagsEnroc[7] ^= PotEnrocarBlancCurt;
	FlagsEnroc[56] ^= PotEnrocarNegreLlarg;
	FlagsEnroc[60] ^= (PotEnrocarNegreLlarg | PotEnrocarNegreCurt);
	FlagsEnroc[63] ^= PotEnrocarNegreCurt;
}

void moviment_a_texte_simple(Moviment m, char *s)
{
	s[0] = 97 + Origen(m) % 8;
	s[1] = Origen(m) / 8 + 49;
	s[2] = 97 + Desti(m) % 8;
	s[3] = Desti(m) / 8 + 49;
	if (EsPromocio(m)) {
		switch (PecaPromocio(m, blanques)) {
		case Dama: s[4] = 'q'; s[5] = 0; break;
		case Torre: s[4] = 'r'; s[5] = 0; break;
		case Caball: s[4] = 'n'; s[5] = 0; break;
		case Alfil: s[4] = 'b'; s[5] = 0; break;
		default:s[4] = 0;
		}
	}
	else
		s[4] = 0;
}

Bitboard AtacsRei[64];
Bitboard AtacsCaball[64];
Bitboard AtacsPeo[2][64];

Bitboard Files[8];
Bitboard Columnes[8];

Bitboard PrecalculTorre[64];
Bitboard PrecalculAlfil[64];
Bitboard Direccio[64][64];
Bitboard CasellesDavant[2][64];
Bitboard Entre[64][64];
Bitboard FilesDavant[2][8] = { {
		0xffffffffffffff00,
		0xffffffffffff0000,
		0xffffffffff000000,
		0xffffffff00000000,
		0xffffff0000000000,
		0xffff000000000000,
		0xff00000000000000,
		0x0000000000000000,
	}, {
		0x0000000000000000,
		0x00000000000000ff,
		0x000000000000ffff,
		0x0000000000ffffff,
		0x00000000ffffffff,
		0x000000ffffffffff,
		0x0000ffffffffffff,
		0x00ffffffffffffff
	} };
Bitboard FilesAdjacentsBB[8] = {
	0x0202020202020202,
	0x0505050505050505,
	0x0a0a0a0a0a0a0a0a,
	0x1414141414141414,
	0x2828282828282828,
	0x5050505050505050,
	0xa0a0a0a0a0a0a0a0,
	0x4040404040404040
};
Bitboard AtacsdePeons[2][64];
Bitboard AvancPeonsPassats[2][64];

void precalcul_bitboards(){
	representacio_peces[PeoBlanc] = 'P';
	representacio_peces[CaballBlanc] = 'C';
	representacio_peces[AlfilBlanc] = 'A';
	representacio_peces[TorreBlanca] = 'T';
	representacio_peces[DamaBlanca] = 'D';
	representacio_peces[ReiBlanc] = 'R';

	representacio_peces[CasellaBuida] = '_';

	representacio_peces[PeoNegre] = 'p';
	representacio_peces[CaballNegre] = 'c';
	representacio_peces[AlfilNegre] = 'a';
	representacio_peces[TorreNegre] = 't';
	representacio_peces[DamaNegre] = 'd';
	representacio_peces[ReiNegre] = 'r';

	int x, y, mida;
	int r, f, color, casella;
	Bitboard b;

	MagicTAttacks[0] = MagicTTable;
	MagicAAttacks[0] = MagicATable;

	Files[0] = FILA1;
	Columnes[0] = COLUMNAA;

	for (x = 1; x < 8; x++) {
		Columnes[x] = Columnes[x - 1] << 1;  //columnes
		Files[x] = Files[x - 1] << 8;  //files
	}

	for (x = 0; x < 64; x++) {

		AtacsRei[x] = 0;

		for (y = 0; y < 64; y++) {
			if (Distancia(x, y) == 1)
				AtacsRei[x] |= Bit(y);
			if (x != y) {
				if (Distancia(x, y) <= 2) {
					if (abs(COL(x) - COL(y)) + abs(ROW(x) - ROW(y)) == 3) AtacsCaball[x] |= Bit(y);
				}
			}
		}

		PrecalculTorre[x] = (Columnes[COL(x)] | Files[ROW(x)]) ^ Bit(x);

		MagicTShift[x] = 64 - popcount(MagicTMask[x]);
		MagicAShift[x] = 64 - popcount(MagicAMask[x]);

		//magics alfils
		b = mida = 0;
		do {
			mida++;
			b = (b - MagicAMask[x]) & MagicAMask[x];
		} while (b);

		if (x < 63)
			MagicAAttacks[x + 1] = MagicAAttacks[x] + mida;

		//magics torre
		b = mida = 0;
		do {
			mida++;
			b = (b - MagicTMask[x]) & MagicTMask[x];
		} while (b);

		if (x < 63)
			MagicTAttacks[x + 1] = MagicTAttacks[x] + mida;
	}

	for (x = 0; x < 64; x++) {
		PrecalculAlfil[x] = AtacsAlfil(x, 0);
	}

	for (x = 0; x < 64; x++) {
		for (b = PrecalculAlfil[x]; b != 0; b &= b - 1) {
			y = lsb(b);
			Direccio[x][y] = PrecalculAlfil[x] & PrecalculAlfil[y];
		}
		for (b = PrecalculTorre[x]; b != 0; b &= b - 1) {
			y = lsb(b);
			Direccio[x][y] = PrecalculTorre[x] & PrecalculTorre[y];
		}
	}

	//Bitboard Entre dos caselles
	for (x = 0; x < 64; x++) {
		for (y = 0; y < 64; y++) {
			Entre[x][y] = 0;
			if (PrecalculAlfil[x] & Bit(y)) {
				Entre[x][y] = (AtacsAlfil(x, Bit(y)) & AtacsAlfil(y, Bit(x)));
			}
			if (PrecalculTorre[x] & Bit(y)) {
				Entre[x][y] = (AtacsTorre(x, Bit(y)) & AtacsTorre(y, Bit(x)));
			}
		}
	}

	for (color = 0; color < 2; color++)
		for (casella = 0; casella < 64; casella++)
		{
			CasellesDavant[color][casella] = FilesDavant[color][ROW(casella)] & Columnes[COL(casella)];  //les caselles per les que passarà el peó quan avançi
			AtacsdePeons[color][casella] = FilesDavant[color][ROW(casella)] & FilesAdjacentsBB[COL(casella)];  //les caselles que ataca ara i quan avanci
			AvancPeonsPassats[color][casella] = CasellesDavant[color][casella] | AtacsdePeons[color][casella];   //les caselles on no hi ha d'haver cap peo contrari pq es consideri passat
			AtacsPeo[color][casella] = AtacsdePeons[color][casella] & ((ROW(casella) > 0 ? Files[ROW(casella) - 1] : 0) | (ROW(casella) < 7 ? Files[ROW(casella) + 1] : 0));
		}
}

void calcular_atacs_escac(tss * RESTRICT ss) {
	Estat_Analisis *estat = ss->estat_actual;
	Bitboard a, b, rei;
	const Bitboard ocupat = TTotespeces;
	estat->destinsescac = 0; //no inclou caselles on pot anar rei
	int numescacs = 0;
	estat->tampocpotanarrei = 0;
	int n;

	estat->atacscaballs[blanques] = estat->atacsalfils[blanques] = estat->atacstorres[blanques] = estat->atacsdames[blanques] = 0;
	estat->atacscaballs[negres] = estat->atacsalfils[negres] = estat->atacstorres[negres] = estat->atacsdames[negres] = 0;

	if (estat->mou == blanques) {
		//atacs blanques
#define jo blanques
		//peons menjar
		estat->atacs[jo] = estat->atacspeons[jo] = ((Tpeons(jo) & (~COLUMNAH)) << 9) | ((Tpeons(jo) & (~COLUMNAA)) << 7); //aquesta línia canvia en copiar-la cap a les negres
		estat->atacs[jo] |= AtacsRei[lsb(Trei(jo))];
		for (a = Tcaballs(jo); a != 0; a &= a - 1) {
			b = AtacsCaball[lsb(a)];
			estat->atacs[jo] |= b;
			estat->atacscaballs[jo] |= b;
		}
		for (a = Talfils(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsAlfil(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsalfil[jo][n] = b;
			estat->atacsalfils[jo] |= b;
		}
		for (a = Ttorres(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsTorre(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacstorre[jo][n] = b;
			estat->atacstorres[jo] |= b;
		}
		for (a = Tdames(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsDama(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsdama[jo][n] = b;
			estat->atacsdames[jo] |= b;
		}
#undef jo
		rei = Trei(blanques);
		//atacs de les negres. mirar si fa escac
#define jo negres
		b = ((Tpeons(jo) & (~COLUMNAH)) >> 7) | ((Tpeons(jo) & (~COLUMNAA)) >> 9); //aquesta línia canvia en copiar-la cap a les negres
		estat->atacs[jo] = estat->atacspeons[jo] = b;
		if (b & rei) {
			numescacs++;
			if (COL(lsb(rei)) > 0 && ss->tau.c[lsb(rei) + 7] == PeoNegre) //aquesta línia canvia en copiar-la cap a les negres
				//if (COL(lsb(rei))>0 && (Tpeons(negres) & Bit(lsb(rei)+7))) //més lent
				estat->destinsescac = Bit(lsb(rei) + 7); //aquesta línia canvia en copiar-la cap a les negres
			else
				estat->destinsescac = Bit(lsb(rei) + 9); //aquesta línia canvia en copiar-la cap a les negres
		}
		estat->atacs[jo] |= AtacsRei[lsb(Trei(jo))];
		for (a = Tcaballs(jo); a != 0; a &= a - 1) {
			b = AtacsCaball[lsb(a)];
			estat->atacs[jo] |= b;
			estat->atacscaballs[jo] |= b;
			if (b & rei) {
				estat->destinsescac = Bit(lsb(a));
				numescacs++;
			}
		}
		for (a = Talfils(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsAlfil(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsalfil[jo][n] = b;
			estat->atacsalfils[jo] |= b;
			if (b & rei) {
				estat->destinsescac |= Entre[lsb(a)][lsb(rei)] | Bit(lsb(a));  //per parar l'escac, pot anar una peça a qualsevol d'aquestes caselles
				estat->tampocpotanarrei = PrecalculAlfil[lsb(a)];  //no pot anar a la casella controlada per l'alfil que està fent escac (darrera del rei) 
				numescacs++;
			}
		}
		for (a = Ttorres(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsTorre(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacstorre[jo][n] = b;
			estat->atacstorres[jo] |= b;
			if (b & rei) {
				estat->destinsescac |= Entre[lsb(a)][lsb(rei)] | Bit(lsb(a));
				estat->tampocpotanarrei |= PrecalculTorre[lsb(a)];
				numescacs++;
			}
		}
		for (a = Tdames(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsDama(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsdama[jo][n] = b;
			estat->atacsdames[jo] |= b;
			if (b & rei) {
				estat->destinsescac |= Entre[lsb(a)][lsb(rei)] | Bit(lsb(a));
				if (COL(lsb(rei)) == COL(lsb(a)) || ROW(lsb(rei)) == ROW(lsb(a)))
					estat->tampocpotanarrei |= PrecalculTorre[lsb(a)];
				else
					estat->tampocpotanarrei |= PrecalculAlfil[lsb(a)];
				numescacs++;
			}
		}
#undef jo
	}
	else {
		//atacs negres
#define jo negres
		//peons menjar
		estat->atacs[jo] = estat->atacspeons[jo] = ((Tpeons(jo) & (~COLUMNAH)) >> 7) | ((Tpeons(jo) & (~COLUMNAA)) >> 9); //diferent
		estat->atacs[jo] |= AtacsRei[lsb(Trei(jo))];
		for (a = Tcaballs(jo); a != 0; a &= a - 1) {
			b = AtacsCaball[lsb(a)];
			estat->atacs[jo] |= b;
			estat->atacscaballs[jo] |= b;
		}
		for (a = Talfils(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsAlfil(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsalfil[jo][n] = b;
			estat->atacsalfils[jo] |= b;
		}
		for (a = Ttorres(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsTorre(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacstorre[jo][n] = b;
			estat->atacstorres[jo] |= b;
		}
		for (a = Tdames(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsDama(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsdama[jo][n] = b;
			estat->atacsdames[jo] |= b;
		}
#undef jo
		rei = Trei(negres);
		//atacs de les negres. mirar si fa escac
#define jo blanques
		b = ((Tpeons(jo) & (~COLUMNAH)) << 9) | ((Tpeons(jo) & (~COLUMNAA)) << 7); //diferent
		estat->atacs[jo] = estat->atacspeons[jo] = b;
		if (b & rei) {
			numescacs++;
			if (COL(lsb(rei)) > 0 && ss->tau.c[lsb(rei) - 9] == PeoBlanc) //diferent
				//if (COL(lsb(rei))>0 && (Tpeons(blanques) & Bit(lsb(rei)-9))) //més lent
				estat->destinsescac = Bit(lsb(rei) - 9); //diferent
			else
				estat->destinsescac = Bit(lsb(rei) - 7); //diferent
		}
		estat->atacs[jo] |= AtacsRei[lsb(Trei(jo))];
		for (a = Tcaballs(jo); a != 0; a &= a - 1) {
			b = AtacsCaball[lsb(a)];
			estat->atacs[jo] |= b;
			estat->atacscaballs[jo] |= b;
			if (b & rei) {
				estat->destinsescac = Bit(lsb(a));
				numescacs++;
			}
		}
		for (a = Talfils(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsAlfil(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsalfil[jo][n] = b;
			estat->atacsalfils[jo] |= b;
			if (b & rei) {
				estat->destinsescac |= Entre[lsb(a)][lsb(rei)] | Bit(lsb(a));
				estat->tampocpotanarrei = PrecalculAlfil[lsb(a)];
				numescacs++;
			}
		}
		for (a = Ttorres(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsTorre(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacstorre[jo][n] = b;
			estat->atacstorres[jo] |= b;
			if (b & rei) {
				estat->destinsescac |= Entre[lsb(a)][lsb(rei)] | Bit(lsb(a));
				estat->tampocpotanarrei |= PrecalculTorre[lsb(a)];
				numescacs++;
			}
		}
		for (a = Tdames(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsDama(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsdama[jo][n] = b;
			estat->atacsdames[jo] |= b;
			if (b & rei) {
				estat->destinsescac |= Entre[lsb(a)][lsb(rei)] | Bit(lsb(a));
				if (COL(lsb(rei)) == COL(lsb(a)) || ROW(lsb(rei)) == ROW(lsb(a)))
					estat->tampocpotanarrei |= PrecalculTorre[lsb(a)];
				else
					estat->tampocpotanarrei |= PrecalculAlfil[lsb(a)];
				numescacs++;
			}
		}
#undef jo
	}
	if (numescacs > 1)
		estat->destinsescac = 0; //amb això pot saber que és escac doble
	estat->escac = !!numescacs;
}

void calcular_atacs_noescac(tss * RESTRICT ss) {
	Estat_Analisis *estat = ss->estat_actual;
	Bitboard a, b, rei;
	const Bitboard ocupat = TTotespeces;
	estat->destinsescac = 0; //no inclou caselles on pot anar rei
	int numescacs = 0;
	estat->tampocpotanarrei = 0;
	int n;

	estat->atacscaballs[blanques] = estat->atacsalfils[blanques] = estat->atacstorres[blanques] = estat->atacsdames[blanques] = 0;
	estat->atacscaballs[negres] = estat->atacsalfils[negres] = estat->atacstorres[negres] = estat->atacsdames[negres] = 0;

	if (estat->mou == blanques) {
		//atacs blanques
#define jo blanques
		//peons menjar
		estat->atacs[jo] = estat->atacspeons[jo] = ((Tpeons(jo) & (~COLUMNAH)) << 9) | ((Tpeons(jo) & (~COLUMNAA)) << 7); //aquesta línia canvia en copiar-la cap a les negres
		estat->atacs[jo] |= AtacsRei[lsb(Trei(jo))];
		for (a = Tcaballs(jo); a != 0; a &= a - 1) {
			b = AtacsCaball[lsb(a)];
			estat->atacs[jo] |= b;
			estat->atacscaballs[jo] |= b;
		}
		for (a = Talfils(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsAlfil(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsalfil[jo][n] = b;
			estat->atacsalfils[jo] |= b;
		}
		for (a = Ttorres(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsTorre(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacstorre[jo][n] = b;
			estat->atacstorres[jo] |= b;
		}
		for (a = Tdames(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsDama(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsdama[jo][n] = b;
			estat->atacsdames[jo] |= b;
		}
#undef jo
		rei = Trei(blanques);
		//atacs de les negres. mirar si fa escac
#define jo negres
		b = ((Tpeons(jo) & (~COLUMNAH)) >> 7) | ((Tpeons(jo) & (~COLUMNAA)) >> 9); //aquesta línia canvia en copiar-la cap a les negres
		estat->atacs[jo] = estat->atacspeons[jo] = b;
		estat->atacs[jo] |= AtacsRei[lsb(Trei(jo))];
		for (a = Tcaballs(jo); a != 0; a &= a - 1) {
			b = AtacsCaball[lsb(a)];
			estat->atacs[jo] |= b;
			estat->atacscaballs[jo] |= b;
		}
		for (a = Talfils(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsAlfil(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsalfil[jo][n] = b;
			estat->atacsalfils[jo] |= b;
		}
		for (a = Ttorres(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsTorre(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacstorre[jo][n] = b;
			estat->atacstorres[jo] |= b;
		}
		for (a = Tdames(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsDama(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsdama[jo][n] = b;
			estat->atacsdames[jo] |= b;
		}
#undef jo
	}
	else {
		//atacs negres
#define jo negres
		//peons menjar
		estat->atacs[jo] = estat->atacspeons[jo] = ((Tpeons(jo) & (~COLUMNAH)) >> 7) | ((Tpeons(jo) & (~COLUMNAA)) >> 9); //diferent
		estat->atacs[jo] |= AtacsRei[lsb(Trei(jo))];
		for (a = Tcaballs(jo); a != 0; a &= a - 1) {
			b = AtacsCaball[lsb(a)];
			estat->atacs[jo] |= b;
			estat->atacscaballs[jo] |= b;
		}
		for (a = Talfils(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsAlfil(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsalfil[jo][n] = b;
			estat->atacsalfils[jo] |= b;
		}
		for (a = Ttorres(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsTorre(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacstorre[jo][n] = b;
			estat->atacstorres[jo] |= b;
		}
		for (a = Tdames(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsDama(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsdama[jo][n] = b;
			estat->atacsdames[jo] |= b;
		}
#undef jo
		rei = Trei(negres);
		//atacs de les negres. mirar si fa escac
#define jo blanques
		b = ((Tpeons(jo) & (~COLUMNAH)) << 9) | ((Tpeons(jo) & (~COLUMNAA)) << 7); //diferent
		estat->atacs[jo] = estat->atacspeons[jo] = b;
		estat->atacs[jo] |= AtacsRei[lsb(Trei(jo))];
		for (a = Tcaballs(jo); a != 0; a &= a - 1) {
			b = AtacsCaball[lsb(a)];
			estat->atacs[jo] |= b;
			estat->atacscaballs[jo] |= b;
		}
		for (a = Talfils(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsAlfil(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsalfil[jo][n] = b;
			estat->atacsalfils[jo] |= b;
		}
		for (a = Ttorres(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsTorre(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacstorre[jo][n] = b;
			estat->atacstorres[jo] |= b;
		}
		for (a = Tdames(jo), n = 0; a != 0; a &= a - 1, n++) {
			b = AtacsDama(lsb(a), ocupat);
			estat->atacs[jo] |= estat->atacsdama[jo][n] = b;
			estat->atacsdames[jo] |= b;
		}
#undef jo
	}
}

void calcular_atacs(tss * RESTRICT ss) {
	if (ss->estat_actual->escac)
		calcular_atacs_escac(ss);
	else
		calcular_atacs_noescac(ss);
}

Bitboard marcar_clavats(tss * RESTRICT ss, bool trobar_pins, int color){
	Bitboard r = 0;
	const int jugador = color ^ trobar_pins;
	const int ell = oponent(jugador);
	const int rei = lsb(Trei(ell));
	Bitboard b = ((Ttorres(jugador) | Tdames(jugador)) & PrecalculTorre[rei]) |
		((Talfils(jugador) | Tdames(jugador)) & PrecalculAlfil[rei]);
	for (; b != 0; b &= b - 1) {
		Bitboard c = (Entre[lsb(b)][rei]) & TTotespeces;
		if (!VarisBits(c) && (c & Tpeces(color)))
			r |= c;
	}
	return r;
}

void perft(tss * RESTRICT ss, int n) {
	nivellactual++;
	calcular_atacs(ss);

	ss->estat_actual->clavats = marcar_clavats(ss, 1, ss->estat_actual->mou);
	if (n > 1)
		ss->estat_actual->descoberts = marcar_clavats(ss, 0, ss->estat_actual->mou);
	const int jo = ss->estat_actual->mou;
	MovList *ml = &ss->estat_actual->moviments[0];
	ss->mov = ml;
	if (ss->estat_actual->escac) {
		if (jo == blanques)
			generar_evasions<blanques>(ss);
		else
			generar_evasions<negres>(ss);
	}
	else {
		generar_moviments(ss);
	}
	ml = &ss->estat_actual->moviments[0];
	while (ml->moviment) {
		if (jugada_ilegal(ss, ml->moviment)) {
			ml++;
			continue;
		}
		if (n <= 1)
			numjugadesperft++;
		else {
			bool jugadaesescac; jugadaesescac = es_escac_bo(ss, ml->moviment);
			fer_moviment(ss, ml->moviment);
			ss->estat_actual++;
			ss->estat_actual->escac = jugadaesescac;
			perft(ss, n - 1);
			desfer_moviment(ss, ss->estat_actual->moviment_previ);
		}
		ml++;
	}
	nivellactual--;
}


void divide(tss * RESTRICT ss, int n) {
	long long ant = 0;
	char jugada[5];
	nivellactual++;
	calcular_atacs(ss);
	ss->estat_actual->clavats = marcar_clavats(ss, 1, ss->estat_actual->mou);
	if (n > 1)
		ss->estat_actual->descoberts = marcar_clavats(ss, 0, ss->estat_actual->mou);
	const int jo = ss->estat_actual->mou;
	ss->mov = &ss->estat_actual->moviments[0];
	if (ss->estat_actual->escac) {
		if (jo == blanques)
			generar_evasions<blanques>(ss);
		else
			generar_evasions<negres>(ss);
	}
	else {
		generar_moviments(ss);
	}
	MovList *ml;
	ml = &ss->estat_actual->moviments[0];
	while (ml->moviment) {
		if (jugada_ilegal(ss, ml->moviment)) {
			ml++;
			continue;
		}
		if (n <= 1)
			numjugadesperft++;
		else {
			bool jugadaesescac; jugadaesescac = es_escac_bo(ss, ml->moviment);
			fer_moviment(ss, ml->moviment);
			ss->estat_actual++;
			ss->estat_actual->escac = jugadaesescac;
			perft(ss, n - 1);
			desfer_moviment(ss, ss->estat_actual->moviment_previ);
		}

		moviment_a_texte_simple(ml->moviment, jugada);
		afout("%s %lli\n", jugada, numjugadesperft - ant);
		ant = numjugadesperft;
		ml++;
	}
	nivellactual--;
}

//http://alienryderflex.com/quicksort/
//  quickSort
//
//  This public-domain C implementation by Darel Rex Finley.
//
//  * This function assumes it is called with valid parameters.
//
//  * Example calls:
//    quickSort(&myArray[0],5); // sorts elements 0, 1, 2, 3, and 4
//    quickSort(&myArray[3],5); // sorts elements 3, 4, 5, 6, and 7

void quickSortJugades(Estat_Analisis *estat_actual) {
#define  MAX_LEVELS  300

	int  beg[MAX_LEVELS], end[MAX_LEVELS], i = 0, L, R, swap;
	Moviment mov;
	int piv;
	MovList *lm = estat_actual->moviments;

	beg[0] = 0;
	end[0] = estat_actual->fi - lm;
	while (i >= 0) {
		L = beg[i]; R = end[i] - 1;
		if (L < R) {
			piv = lm[L].puntuacio;
			mov = lm[L].moviment;
			while (L < R) {
				while (lm[R].puntuacio <= piv && L < R) R--; if (L < R) {
					lm[L].moviment = lm[R].moviment;
					lm[L++].puntuacio = lm[R].puntuacio;
				}
				while (lm[L].puntuacio >= piv && L < R) L++; if (L < R) {
					lm[R].moviment = lm[L].moviment;
					lm[R--].puntuacio = lm[L].puntuacio;
				}
			}
			lm[L].moviment = mov;
			lm[L].puntuacio = piv;
			beg[i + 1] = L + 1; end[i + 1] = end[i]; end[i++] = L;
			if (end[i] - beg[i] > end[i - 1] - beg[i - 1]) {
				swap = beg[i]; beg[i] = beg[i - 1]; beg[i - 1] = swap;
				swap = end[i]; end[i] = end[i - 1]; end[i - 1] = swap;
			}
		}
		else {
			i--;
		}
	}
}

void OrdenacioEstableJugades(Estat_Analisis *estat_actual){
	int i, j;
	MovList *lm = estat_actual->moviments;
	MovList temp;
	int num = estat_actual->fi - lm;
	for (i = 1; i < num; i++)
		if (lm[i - 1].puntuacio < lm[i].puntuacio) {
			temp = lm[i];
			lm[i] = lm[i - 1];
			for (j = i - 1; j > 0 && lm[j - 1].puntuacio < temp.puntuacio; j--)
				lm[j] = lm[j - 1];
			lm[j] = temp;
		}
}

int color_casella(int sq) {
	return (sq + ROW(sq + 8)) & 1;
}

Bitboard atacants(tss * RESTRICT ss, uint8_t casella, Bitboard occ) {
	return (AtacsCaball[casella] & (Tcaballs(blanques) | Tcaballs(negres)))
		| (AtacsRei[casella] & (Trei(blanques) | Trei(negres)))
		| (AtacsAlfil(casella, occ) & (Talfils(blanques) | Talfils(negres) | Tdames(blanques) | Tdames(negres)))
		| (AtacsTorre(casella, occ) & (Ttorres(blanques) | Ttorres(negres) | Tdames(blanques) | Tdames(negres)))
		//| (AtacsDama(casella, occ) & (Tdames(blanques) | Tdames(negres)))
		| (AtacsPeo[negres][casella] & Tpeons(blanques))
		| (AtacsPeo[blanques][casella] & Tpeons(negres));
}

Bitboard atacs_desde(uint8_t p, int casella, Bitboard occ) {
	switch (peca_sense_color(p))
	{
	case AlfilBlanc: return AtacsAlfil(casella, occ);
	case TorreBlanca: return AtacsTorre(casella, occ);
	case DamaBlanca: return AtacsDama(casella, occ);
	case ReiBlanc: return AtacsRei[casella];
	case CaballBlanc: return AtacsCaball[casella];
	}
	if (p == PeoBlanc)
		return AtacsPeo[blanques][casella];
	else
		return AtacsPeo[negres][casella];
}

int min_atacant(tss * RESTRICT ss, e_colors jo, int to, Bitboard Atacantsmouen,
	Bitboard& ocupat, Bitboard& atacants) {

	Bitboard b = Atacantsmouen & Tpeces(afegir_color_a_peca(PeoBlanc, jo));
	if (!b)
		goto ma_caball;
	ocupat ^= b & ~(b - 1);
	//afegir xray
	atacants |= AtacsAlfil(to, ocupat) & (Tpeces(Alfil) | Tpeces(Dama));
	atacants &= ocupat;
	return PeoBlanc;

ma_caball:
	b = Atacantsmouen & Tpeces(afegir_color_a_peca(CaballBlanc, jo));
	if (!b)
		goto ma_alfil;
	ocupat ^= b & ~(b - 1);
	atacants &= ocupat;
	return CaballBlanc;

ma_alfil:
	b = Atacantsmouen & Tpeces(afegir_color_a_peca(AlfilBlanc, jo));
	if (!b)
		goto ma_torre;
	ocupat ^= b & ~(b - 1);
	atacants |= AtacsAlfil(to, ocupat) & (Tpeces(Alfil) | Tpeces(Dama));
	atacants &= ocupat;
	return AlfilBlanc;

ma_torre:
	b = Atacantsmouen & Tpeces(afegir_color_a_peca(TorreBlanca, jo));
	if (!b)
		goto ma_dama;
	ocupat ^= b & ~(b - 1);
	atacants |= AtacsTorre(to, ocupat) & (Tpeces(Torre) | Tpeces(Dama));
	atacants &= ocupat;
	return TorreBlanca;

ma_dama:
	b = Atacantsmouen & Tpeces(afegir_color_a_peca(DamaBlanca, jo));
	if (!b)
		return Rei;
	ocupat ^= b & ~(b - 1);
	atacants |= AtacsAlfil(to, ocupat) & (Tpeces(Alfil) | Tpeces(Dama));
	atacants |= AtacsTorre(to, ocupat) & (Tpeces(Torre) | Tpeces(Dama));
	atacants &= ocupat;
	return DamaBlanca;
}

void setHistory(tss * RESTRICT ss, Moviment m, bool finsjugada, int puntuacio) {
	Estat_Analisis *estat_actual = ss->estat_actual;
	if (PuntuacioHistorial(ss->tau.c[Origen(m)], Desti(m)) + puntuacio < MaxPuntHistorial) {
		SumaHistorial(ss->tau.c[Origen(m)], Desti(m), puntuacio);
#ifdef GUARDARLOG
		sprintf(logstring, "sumhist %i %i", m, puntuacio);
		glog(logstring);
#endif
	}

	if (finsjugada) {
		MovList *ml;
		ml = &estat_actual->moviments[0];
		while (ml->moviment) {
			if (ml->moviment == JugadaInvalida2) {
				ml++;
				continue;
			}
			if (ml->moviment == m) {
				return;
			}
			if (ss->tau.c[Desti(ml->moviment)] == CasellaBuida
				&& !EsPromocio(ml->moviment)
				&& !EsAlPas(ml->moviment)) {
				if (PuntuacioHistorial(ss->tau.c[Origen(ml->moviment)], Desti(ml->moviment)) - puntuacio > -MaxPuntHistorial) {
					SumaHistorial(ss->tau.c[Origen(ml->moviment)], Desti(ml->moviment), -puntuacio);
#ifdef GUARDARLOG
					sprintf(logstring, "sumhist %i %i", ml->moviment, -puntuacio);
					glog(logstring);
#endif
				}
			}
			ml++;
		}
	}

}

char representacio_peces[ReiNegre + 1];
void mostra_taulell(tss * RESTRICT ss) {
	int x;
	int y;
	for (y = 7; y >= 0; y--) {
		for (x = 0; x <= 7; x++)
			afout("%c", representacio_peces[ss->tau.c[y * 8 + x]]);
		afout("\n");
	}
}

int8_t reduccio_lmr[64][maxjugadeslmr];

int MovimentsFutils[MaxMovimentsFutils] =
{ 2, 3, 5, 7, 11, 16, 22, 29, 35, 43, 51, 60, 65, 70, 75, 80, 85, 90, 95, 105 };

void inicialitzar_lmr(){
	int p, j;
	double idx;
	for (p = 1; p < 64; p++) {
		idx = 2 + 8 / (float)p;
		for (j = 0; j < maxjugadeslmr; j++) {
			reduccio_lmr[p][j] = 1 + (j >= idx) + (j >= 4 * idx) + (j >= 8 * idx) + (p > 8) + (p > 15);
		}
		reduccio_lmr[p][0] = 0;
		reduccio_lmr[p][1] = 0;
		if (reduccio_lmr[p][2] > 1)
			reduccio_lmr[p][1] = 1;
}

	reduccio_lmr[9][2] = 1;
	reduccio_lmr[9][3] = 2;
	reduccio_lmr[9][4] = 2;
	reduccio_lmr[9][5] = 2;

	reduccio_lmr[10][2] = 1;
	reduccio_lmr[10][3] = 2;
	reduccio_lmr[10][4] = 2;
	reduccio_lmr[10][5] = 2;

	reduccio_lmr[11][2] = 1;
	reduccio_lmr[11][3] = 2;
	reduccio_lmr[11][4] = 2;
}


