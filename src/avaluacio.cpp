#include "definicions.h"
#include "analisis.h"
#include <stdlib.h>
#include <stdio.h>

#include "avaluacio.h"
#include "debug.h"
#include "utils.h"
#include "finals.h"
#include "hash.h"
#include <string.h>

using namespace std;

#ifdef WINDOWS
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)
#endif

#ifdef LINUX
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

int valorpeca[ReiNegre + 1];
int valorpecasee[ReiNegre + 1]; //per SEE
int valorpecaseeambRei[ReiNegre + 1]; //per SEE
int avaluacio_rapida();
int BonusPerQuiMou;

int valorposiciopeonspassats[2][64];
int bonificacio_base_peons_passats_potser_no_parables;

int penalitzacio_rei_contrari_proper_peons_passats_final[8];
int bonificacio_rei_proper_peons_passats_final[8];
#define bonificacio_rei_parant_peo_passat_final FerPun(0, 15)
#define penalitzacio_dama_parant_peo_passat_final FerPun(10, 13)
#define PenalitzacioExtraPeoPassatNoControlatFinal FerPun(-2, 17)
int BonificacioPassatAmbSuport;
int PenalitzacioTorreBloquejada;
int BonusTorreColumnaSemiOberta;
int BonusTorreColumnaOberta;
int BonusTorresDoblades;
int BonusTorre6a;
int BonusTorreTallaRei7a;
int BonusTorreMiraDama;
int BonusParellaAlfils;
int BonusParellaAlfilsFinal;
int BonusParellaAlfilsNoCanviables;
int PenalitzacioParellaAlfilsSiMoltsPeons;
int PenalitzacioAlfilMateixColor1Peo;
int PenalitzacioAlfilBloquejatPeoCentral, PenalitzacioAlfilBloquejatPeoCentral2;
int AlfilClavantCaballvsDama;
int BonusAlfilPerPeonsEnDosFlancs;
int BonusAlfilNoPotSerExpulsatPerPeo[8];
int BonusAlfilSuportatPerPeo;
int MinValorCaballsPerBonusvsAlfilsBons;
int BonusParellaAlfilsvsCaballsBons;
int BonusParellaAlfilsvsCaballsBonsFinal;
int PenalitzacioPeoDebil[8]; //per columnes

#define PenalitzacioPecaAtacaCasellaDavantDebil FerPun(2, 3) //(2, 5) //no: bo a ràpides (4, 5)
#define BonificacioDebilnoAtacadaNiOcupada FerPun(3, 3)
#define PenalitzacioPecaAtacaDebil FerPun(3, 8) //(3, 8) //no: (3, 10), no està clar (3, 6)
#define PenalitzacioPecaOcupaDavantDebil FerPun(11, 11)  //(7,11)
int PenalitzacioPeoOcupaDavantDebil;
int penalizacio_peo_debil_costat_columna_oberta = FerPun(3, 3);

int PenalitzacioPeoDoblatAillat;
int Aillat[2][8];
int PenalitzacioPeoNoSuportat;

int BonusMenysPeonsCentralsSiMesTorresoAlfils[9];
//si en final el rei no està prop de la columna més propera que té peons de qualsevol color
//** potser no és bo per final on hi ha dames
int PenalitzacioDistanciaReiColumnaPeonsAux[8];
int PenalitzacioDistanciaReiColumnaPeons[8][256]; //columna, estructura peons dels dos colors juntades amb or
int PenalitzacioIllesPeons[256];

int MenorDarreraPeo;

int BonusCaballSuportatPerPeo;
int BonusCaballNoPotSerExpulsatPerPeo[8];
int PenalitzacioCaballAtrapatPeons;
int PenalitzacioCaballBloquejaPeoC;

int PenalitzacioReiCentreSenseEnroc;
int PenalitzacioTorreICaballposicioOriginal;
int PenalitzacioDiferenciaDesenvolupament[11];

int CompensacioEntreMaterialIMobilitat1 = 15, CompensacioEntreMaterialIMobilitat1Final = 19;
int CompensacioEntreMaterialIMobilitat2 = 25, CompensacioEntreMaterialIMobilitat2Final = 22;
int CompensacioEntreMaterialIMobilitat3 = 45, CompensacioEntreMaterialIMobilitat3Final = 45;

int ValorSimplificacio2;

int vp_peons[64] = { // De cara a el negre, que està avall
	0, 0, 0, 0, 0, 0, 0, 0,
	-7, -2, 1, 6, 6, 1, -2, -7,
	-7, -2, 3, 14, 14, 3, -2, -7,
	-7, -2, 7, 21, 21, 7, -2, -7,
	-7, -2, 7, 14, 14, 7, -2, -7,
	-7, -2, 3, 4, 4, 3, -2, -7,
	-7, -2, 1, 4, 4, 3, -2, -7,
	0, 0, 0, 0, 0, 0, 0, 0 };

int vp_caballs[64];

int valorposicioalfils[64];

int valorposiciotorres[64];

int valorposiciodama[64];

int vp_rei[64];

int mobilitatc[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int mobilitata[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int mobilitatt[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int mobilitatd[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

struct TMaterial taulamaterial[maxdamab][maxtorreb][maxalfilb][maxcaballb][maxdaman][maxtorren][maxalfiln][maxcaballn];

int alfil_per_peons[6];

int caball_per_peons[6];

int caball_i_peons_per_torre[6];

int alfil_i_peons_per_torre[6];

int alfil_i_caball_per_torre_i_peons[6];

int dama_per_torre_caball_i_peons[6];

int dama_per_torre_alfil_i_peons[6];

int dama_i_alfil_per_torres[6] = { FerPun(85, 85), FerPun(85, 85), FerPun(85, 85), FerPun(85, 85), FerPun(85, 85), FerPun(85, 85) };
int dama_i_caball_per_torres[6] = { FerPun(80, 80), FerPun(80, 80), FerPun(80, 80), FerPun(80, 80), FerPun(80, 80), FerPun(80, 80) };

int torre_i_peons_per_dos_alfils[6];

int dama_i_peons_per_torres[6];

int dama_per_torre_alfil_amb_parella_i_peons[6];

int dama_per_torre_alfil_i_caball[6] = { FerPun(30, 30), FerPun(30, 30), FerPun(30, 30), FerPun(30, 30), FerPun(30, 30), FerPun(30, 30) };

#define PesAtacDama 5
#define PesAtacTorre 3
#define PesAtacAlfil 2
#define PesAtacCaball 2

#define EscacContacteDama 9
#define EscacContacteTorre 5
#define EscacDama 3
#define EscacTorre 2
#define EscacAlfil 2
#define EscacCaball 1

// http://talkchess.com/forum/viewtopic.php?t=56191
int Lever[2][8]; //oposat i fila

void precalcul_estructura_peons() {
	int i, j, k, l, m;
	for (i = 0; i < 256; i++) {
		PenalitzacioIllesPeons[i] = 0;
		int columnaant = -1;
		for (j = 0; j < 8; j++){  //per cada bit de i
			if (TestBit(i, j)) {
				if (columnaant != -1 && columnaant != j - 1)
					PenalitzacioIllesPeons[i] += FerPun(0, 2);
				columnaant = j;
			}
		}

		//si en final el rei no està prop de la columna més propera que té peons de qualsevol color
		for (j = 0; j < 8; j++) { //j=columna on estaria el rei
			PenalitzacioDistanciaReiColumnaPeons[j][i] = 0; //per defecte no hi ha penalització
			//si no hi ha peons, no penalització
			if (i == 0)
				continue;
			//si hi ha peons a la columna del rei, no penalització
			if (TestBit(i, j))
				continue;
			//buscar columna esquerra més propera al rei que té peons
			for (k = j - 1; k >= 0; k--)
				if (TestBit(i, k))
					break;
			//buscar columna dreta més propera al rei que té peons
			for (l = j + 1; l <= 7; l++)
				if (TestBit(i, l))
					break;
			//si hi ha peons a les columnes anteriors i posteriors del rei, no hi ha penalització
			if (k >= 0 && l <= 7 && j >= k && j <= l)
				continue;
			if (k >= 0)
				m = j - k;
			else
				m = 99;
			if (l <= 7 && l - j < m)
				m = l - j;
			PenalitzacioDistanciaReiColumnaPeons[j][i] = FerPun(0, PenalitzacioDistanciaReiColumnaPeonsAux[m]);
		}
	}
}

void calcular_imbalanc() {
	//precalcular taula tipusimbalanc
	//taulamaterial[maxdamab][maxtorreb][maxalfilb][maxcaballb][maxdaman][maxtorren][maxalfiln][maxcaballn];
	int db, tb, ab, cb, dn, tn, an, cn;
	for (db = 0; db < maxdamab; db++)
		for (tb = 0; tb < maxtorreb; tb++)
			for (ab = 0; ab < maxalfilb; ab++)
				for (cb = 0; cb < maxcaballb; cb++)
					for (dn = 0; dn < maxdaman; dn++)
						for (tn = 0; tn < maxtorren; tn++)
							for (an = 0; an < maxalfiln; an++)
								for (cn = 0; cn < maxcaballn; cn++) {
									taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = NULL;
									taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = 1;
									taulamaterial[db][tb][ab][cb][dn][tn][an][cn].phase = maxalpha;

									if (db == dn && tb == tn && ab == an + 1 && cb == cn) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = alfil_per_peons;
									}
									if (db == dn && tb == tn && an == ab + 1 && cb == cn)
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = alfil_per_peons;

									if (db == dn && tb == tn && ab == an && cb == cn + 1) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = caball_per_peons;
									}
									if (db == dn && tb == tn && ab == an && cn == cb + 1)
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = caball_per_peons;

									if (db == dn && tb == tn - 1 && ab == an && cb == cn + 1) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = caball_i_peons_per_torre;
									}
									if (db == dn && tn == tb - 1 && ab == an && cn == cb + 1)
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = caball_i_peons_per_torre;

									if (db == dn && tb == tn - 1 && ab == an + 1 && cb == cn) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = alfil_i_peons_per_torre;
									}
									if (db == dn && tn == tb - 1 && an == ab + 1 && cn == cb)
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = alfil_i_peons_per_torre;

									if (db == dn && tb == tn - 1 && ab == an + 1 && cb == cn + 1) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = alfil_i_caball_per_torre_i_peons;
									}
									if (db == dn && tn == tb - 1 && an == ab + 1 && cn == cb + 1)
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = alfil_i_caball_per_torre_i_peons;

									if (dn == db - 1 && tn == tb + 1 && ab == an && cn == cb + 1) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_per_torre_caball_i_peons;
									}
									if (db == dn - 1 && tb == tn + 1 && ab == an && cb == cn + 1)
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_per_torre_caball_i_peons;

									if (dn == db - 1 && tn == tb + 1 && an == ab + 1 && cn == cb) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_per_torre_alfil_i_peons;
									}
									if (db == dn - 1 && tb == tn + 1 && ab == an + 1 && cb == cn)
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_per_torre_alfil_i_peons;

									if (dn == db - 1 && tn == tb + 1 && an == ab + 1 && cn == cb && an == 2) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_per_torre_alfil_amb_parella_i_peons;
									}
									if (db == dn - 1 && tb == tn + 1 && ab == an + 1 && cb == cn && ab == 2)
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_per_torre_alfil_amb_parella_i_peons;



									if (dn == db - 1 && tn == tb + 1 && an == ab + 1 && cn == cb + 1) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_per_torre_alfil_i_caball;
									}
									if (db == dn - 1 && tb == tn + 1 && ab == an + 1 && cb == cn + 1)
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_per_torre_alfil_i_caball;


									if (dn == db - 1 && tn == tb + 2 && ab + cb == an + cn) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_i_peons_per_torres;
									}
									if (db == dn - 1 && tb == tn + 2 && ab + cb == an + cn)
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_i_peons_per_torres;

									if (db == dn && tb == 2 && tn == 1 && an == 2 && ab == 0 && cb == cn) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = torre_i_peons_per_dos_alfils;
									}
									if (db == dn && tn == 2 && tb == 1 && ab == 2 && an == 0 && cb == cn)
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = torre_i_peons_per_dos_alfils;

									if (db == dn && tb == 1 && tn == 0 && an == 2 && ab == 0 && cb == cn) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = torre_i_peons_per_dos_alfils;
									}
									if (db == dn && tn == 1 && tb == 0 && ab == 2 && an == 0 && cb == cn)
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = torre_i_peons_per_dos_alfils;


									if (dn + 1 == db && tn - 2 == tb && ab == an && cn + 1 == cb) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_i_caball_per_torres;
									}
									if (db + 1 == dn && tb - 2 == tn && ab == an && cb + 1 == cn) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_i_caball_per_torres;
									}


									if (dn + 1 == db && tn - 2 == tb && cb == cn && an + 1 == ab) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_i_alfil_per_torres;
									}
									if (db + 1 == dn && tb - 2 == tn && cb == cn && ab + 1 == an) {
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].signe = -1;
										taulamaterial[db][tb][ab][cb][dn][tn][an][cn].imbalanc = dama_i_alfil_per_torres;
									}

									int z = tb * 2 + tn * 2 + db * 4 + dn * 4;
									int phase = 24 - (cb + cn + ab + an + z);
									if (phase > 20)
										phase = 20;  //considerar finals de 2 torres com final absolut
									taulamaterial[db][tb][ab][cb][dn][tn][an][cn].phase = (phase * 256 + (20 / 2)) / 20;
								}
}

#define BaseSeguretatRei -83

#define MulPesAtacs  4 //no: 3, 5
#define MulEscacsContacte 17 //17 //no: 16,18,19,15,13 BO
#define MulEscacsDistancia 19 //no 20 i altres
#define MulAtacsCostat -2 //-2  //no:0
#define MulPeonsDavant 10 //12  //no 8, 13 //14 un pel millor a ràpides o molt poquet millor
#define MulAtacsQuadre 2 //2 ori //1 tunejat
#define MulReiAvancat 42  //42 //no: 46
#define MulNumAtacants 11 //10
#define MulNumDefensant -5  //-5 tunejat //-1 ori //no: -2, -3

int seguretat_final = -1;

int mulpeonsdavantcontrari;
int penalitzaciosemibloqueigcentral;
int penalitzaciosibloqueigcentral;
int minpenalitzacioreiperdisminuirsiunatacant;

void carregar_fitxer_avaluacions() {
	int i, j;

	PenalitzacioTorreICaballposicioOriginal = FerPun(PenalitzacioTorreICaballposicioOriginal, PenalitzacioTorreICaballposicioOriginal);

	valorpeca[PeoBlanc] = FerPun(vpeo, 107); //no(80, 107)
	valorpeca[PeoNegre] = valorpeca[PeoBlanc];
	valorpeca[CaballBlanc] = FerPun(vcaball, 322); valorpeca[CaballNegre] = valorpeca[CaballBlanc];
	valorpeca[AlfilBlanc] = FerPun(valfil, 339); valorpeca[AlfilNegre] = valorpeca[AlfilBlanc];
	valorpeca[TorreBlanca] = FerPun(vtorre, 528); valorpeca[TorreNegre] = valorpeca[TorreBlanca];
	valorpeca[DamaBlanca] = FerPun(vdama, 994); valorpeca[DamaNegre] = valorpeca[DamaBlanca];

	//per SEE, idéntic al normal però alfil=caball pq no descarti canvis entre ells
	for (i = 0; i <= ReiNegre; i++)
		valorpecasee[i] = PunI(valorpeca[i]);
	valorpecasee[PeoBlanc] = vpeosee; valorpecasee[PeoNegre] = valorpecasee[PeoBlanc];
	valorpecasee[CaballBlanc] = vcaballsee; valorpecasee[CaballNegre] = valorpecasee[CaballBlanc];
	valorpecasee[TorreBlanca] = vtorresee; valorpecasee[TorreNegre] = valorpecasee[TorreBlanca];
	valorpecasee[AlfilBlanc] = valorpecasee[CaballBlanc];
	valorpecasee[AlfilNegre] = valorpecasee[CaballBlanc];
	valorpecasee[DamaBlanca] = vdamasee; valorpecasee[DamaNegre] = valorpecasee[DamaBlanca];

	for (i = 0; i <= ReiNegre; i++)
		valorpecaseeambRei[i] = valorpecasee[i];
	valorpecaseeambRei[ReiBlanc] = vreisee;
	valorpecaseeambRei[ReiNegre] = vreisee;

	for (i = 0; i < 64; i++)
		vp_peons[i] = FerPun(vp_peons[i], -3); //-3 valor fixe peons final

	vp_caballs[0] = FerPun(-47, -57);
	vp_caballs[1] = FerPun(-21, -22);
	vp_caballs[2] = FerPun(-14, -20);
	vp_caballs[3] = FerPun(-13, -11);
	vp_caballs[4] = FerPun(-11, -16);
	vp_caballs[5] = FerPun(-14, -20);
	vp_caballs[6] = FerPun(-18, -23);
	vp_caballs[7] = FerPun(-49, -54);
	vp_caballs[8] = FerPun(-27, -28);
	vp_caballs[9] = FerPun(-11, -12);
	vp_caballs[10] = FerPun(-6, -8);
	vp_caballs[11] = FerPun(1, -1);
	vp_caballs[12] = FerPun(1, -2);
	vp_caballs[13] = FerPun(-7, -8);
	vp_caballs[14] = FerPun(-12, -12);
	vp_caballs[15] = FerPun(-25, -27);
	vp_caballs[16] = FerPun(-11, -13);
	vp_caballs[17] = FerPun(-7, -2);
	vp_caballs[18] = FerPun(6, 3);
	vp_caballs[19] = FerPun(13, 10);
	vp_caballs[20] = FerPun(14, 13);
	vp_caballs[21] = FerPun(0, 0);
	vp_caballs[22] = FerPun(0, -2);
	vp_caballs[23] = FerPun(-21, -18);
	vp_caballs[24] = FerPun(4, -3);
	vp_caballs[25] = FerPun(4, 3);
	vp_caballs[26] = FerPun(22, 20);
	vp_caballs[27] = FerPun(20, 19);
	vp_caballs[28] = FerPun(20, 19);
	vp_caballs[29] = FerPun(21, 15);
	vp_caballs[30] = FerPun(5, 2);
	vp_caballs[31] = FerPun(-1, 1);
	vp_caballs[32] = FerPun(1, -2);
	vp_caballs[33] = FerPun(5, 9);
	vp_caballs[34] = FerPun(19, 17);
	vp_caballs[35] = FerPun(28, 19);
	vp_caballs[36] = FerPun(28, 20);
	vp_caballs[37] = FerPun(24, 18);
	vp_caballs[38] = FerPun(4, 4);
	vp_caballs[39] = FerPun(-2, 1);
	vp_caballs[40] = FerPun(-16, -15);
	vp_caballs[41] = FerPun(3, 7);
	vp_caballs[42] = FerPun(11, 8);
	vp_caballs[43] = FerPun(18, 13);
	vp_caballs[44] = FerPun(16, 11);
	vp_caballs[45] = FerPun(8, 8);
	vp_caballs[46] = FerPun(3, 2);
	vp_caballs[47] = FerPun(-17, -15);
	vp_caballs[48] = FerPun(-23, -22);
	vp_caballs[49] = FerPun(-13, -8);
	vp_caballs[50] = FerPun(-2, -3);
	vp_caballs[51] = FerPun(-5, -5);
	vp_caballs[52] = FerPun(-4, -5);
	vp_caballs[53] = FerPun(-2, -3);
	vp_caballs[54] = FerPun(-11, -8);
	vp_caballs[55] = FerPun(-27, -22);
	vp_caballs[56] = FerPun(-52, -48);
	vp_caballs[57] = FerPun(-30, -30);
	vp_caballs[58] = FerPun(-21, -22);
	vp_caballs[59] = FerPun(-19, -18);
	vp_caballs[60] = FerPun(-21, -19);
	vp_caballs[61] = FerPun(-24, -22);
	vp_caballs[62] = FerPun(-30, -31);
	vp_caballs[63] = FerPun(-50, -50);

	int sumaf = -15;
	vp_rei[0] = FerPun(15, -39 + sumaf);
	vp_rei[1] = FerPun(23, -20 + sumaf);
	vp_rei[2] = FerPun(9, -9 + sumaf);
	vp_rei[3] = FerPun(-10, -6 + sumaf);
	vp_rei[4] = FerPun(-7, -6 + sumaf);
	vp_rei[5] = FerPun(8, -9 + sumaf);
	vp_rei[6] = FerPun(28, -20 + sumaf);
	vp_rei[7] = FerPun(16, -39 + sumaf);
	vp_rei[8] = FerPun(3, -22 + sumaf);
	vp_rei[9] = FerPun(7, -3 + sumaf);
	vp_rei[10] = FerPun(-6, 17 + sumaf);
	vp_rei[11] = FerPun(-22, 13 + sumaf);
	vp_rei[12] = FerPun(-27, 13 + sumaf);
	vp_rei[13] = FerPun(-3, 17 + sumaf);
	vp_rei[14] = FerPun(13, -3 + sumaf);
	vp_rei[15] = FerPun(6, -22 + sumaf);
	vp_rei[16] = FerPun(-18, -7 + sumaf);
	vp_rei[17] = FerPun(-28, 15 + sumaf);
	vp_rei[18] = FerPun(-26, 27 + sumaf);
	vp_rei[19] = FerPun(-28, 25 + sumaf);
	vp_rei[20] = FerPun(-31, 25 + sumaf);
	vp_rei[21] = FerPun(-26, 27 + sumaf);
	vp_rei[22] = FerPun(-28, 15 + sumaf);
	vp_rei[23] = FerPun(-20, -7 + sumaf);
	vp_rei[24] = FerPun(-27, 2 + sumaf);
	vp_rei[25] = FerPun(-28, 26 + sumaf);
	vp_rei[26] = FerPun(-29, 26 + sumaf);
	vp_rei[27] = FerPun(-37, 30 + sumaf);
	vp_rei[28] = FerPun(-39, 30 + sumaf);
	vp_rei[29] = FerPun(-29, 26 + sumaf);
	vp_rei[30] = FerPun(-27, 26 + sumaf);
	vp_rei[31] = FerPun(-27, 2 + sumaf);
	vp_rei[32] = FerPun(-31, 2 + sumaf);
	vp_rei[33] = FerPun(-37, 24 + sumaf);
	vp_rei[34] = FerPun(-37, 38 + sumaf);
	vp_rei[35] = FerPun(-52, 39 + sumaf);
	vp_rei[36] = FerPun(-49, 39 + sumaf);
	vp_rei[37] = FerPun(-39, 38 + sumaf);
	vp_rei[38] = FerPun(-38, 24 + sumaf);
	vp_rei[39] = FerPun(-28, 2 + sumaf);
	vp_rei[40] = FerPun(-27, -4 + sumaf);
	vp_rei[41] = FerPun(-36, 20 + sumaf);
	vp_rei[42] = FerPun(-39, 28 + sumaf);
	vp_rei[43] = FerPun(-51, 28 + sumaf);
	vp_rei[44] = FerPun(-48, 28 + sumaf);
	vp_rei[45] = FerPun(-37, 28 + sumaf);
	vp_rei[46] = FerPun(-38, 20 + sumaf);
	vp_rei[47] = FerPun(-29, -4 + sumaf);
	vp_rei[48] = FerPun(-30, -22 + sumaf);
	vp_rei[49] = FerPun(-39, -2 + sumaf);
	vp_rei[50] = FerPun(-39, 6 + sumaf);
	vp_rei[51] = FerPun(-52, 13 + sumaf);
	vp_rei[52] = FerPun(-51, 13 + sumaf);
	vp_rei[53] = FerPun(-39, 6 + sumaf);
	vp_rei[54] = FerPun(-39, -2 + sumaf);
	vp_rei[55] = FerPun(-29, -22 + sumaf);
	vp_rei[56] = FerPun(-31, -38 + sumaf);
	vp_rei[57] = FerPun(-40, -20 + sumaf);
	vp_rei[58] = FerPun(-41, -10 + sumaf);
	vp_rei[59] = FerPun(-49, -6 + sumaf);
	vp_rei[60] = FerPun(-48, -6 + sumaf);
	vp_rei[61] = FerPun(-38, -10 + sumaf);
	vp_rei[62] = FerPun(-40, -20 + sumaf);
	vp_rei[63] = FerPun(-28, -38 + sumaf);

	valorposicioalfils[0] = FerPun(-19, -21);
	valorposicioalfils[1] = FerPun(-8, -11);
	valorposicioalfils[2] = FerPun(-13, -13);
	valorposicioalfils[3] = FerPun(-18, -7);
	valorposicioalfils[4] = FerPun(-16, -7);
	valorposicioalfils[5] = FerPun(-15, -14);
	valorposicioalfils[6] = FerPun(-10, -12);
	valorposicioalfils[7] = FerPun(-20, -22);
	valorposicioalfils[8] = FerPun(-14, -18);
	valorposicioalfils[9] = FerPun(0, -10);
	valorposicioalfils[10] = FerPun(-3, -5);
	valorposicioalfils[11] = FerPun(-10, -4); //(-6, -4)
	valorposicioalfils[12] = FerPun(-6, -4);
	valorposicioalfils[13] = FerPun(-3, -7);
	valorposicioalfils[14] = FerPun(0, -5);
	valorposicioalfils[15] = FerPun(-14, -14);
	valorposicioalfils[16] = FerPun(-4, -10);
	valorposicioalfils[17] = FerPun(2, 0);
	valorposicioalfils[18] = FerPun(5, -4);
	valorposicioalfils[19] = FerPun(-3, -1);
	valorposicioalfils[20] = FerPun(2, 0);
	valorposicioalfils[21] = FerPun(4, -5);
	valorposicioalfils[22] = FerPun(9, 0);
	valorposicioalfils[23] = FerPun(-8, -10);
	valorposicioalfils[24] = FerPun(-10, -11);
	valorposicioalfils[25] = FerPun(0, -6);
	valorposicioalfils[26] = FerPun(5, -2);
	valorposicioalfils[27] = FerPun(3, 2);
	valorposicioalfils[28] = FerPun(2, 1);
	valorposicioalfils[29] = FerPun(5, -3);
	valorposicioalfils[30] = FerPun(1, -7);
	valorposicioalfils[31] = FerPun(-7, -13);
	valorposicioalfils[32] = FerPun(-3, -12);
	valorposicioalfils[33] = FerPun(4, -8);
	valorposicioalfils[34] = FerPun(-1, -3);
	valorposicioalfils[35] = FerPun(2, -1);
	valorposicioalfils[36] = FerPun(-1, 0);
	valorposicioalfils[37] = FerPun(-5, -8);
	valorposicioalfils[38] = FerPun(5, -3);
	valorposicioalfils[39] = FerPun(-3, -16);
	valorposicioalfils[40] = FerPun(-6, -11);
	valorposicioalfils[41] = FerPun(4, -6);
	valorposicioalfils[42] = FerPun(-2, -6);
	valorposicioalfils[43] = FerPun(-6, 3);
	valorposicioalfils[44] = FerPun(-8, -1);
	valorposicioalfils[45] = FerPun(-3, -5);
	valorposicioalfils[46] = FerPun(5, -4);
	valorposicioalfils[47] = FerPun(-11, -9);
	valorposicioalfils[48] = FerPun(-7, -14);
	valorposicioalfils[49] = FerPun(4, -11);
	valorposicioalfils[50] = FerPun(0, -10);
	valorposicioalfils[51] = FerPun(-7, -2);
	valorposicioalfils[52] = FerPun(-7, -4);
	valorposicioalfils[53] = FerPun(-2, -11);
	valorposicioalfils[54] = FerPun(5, -9);
	valorposicioalfils[55] = FerPun(-8, -21);
	valorposicioalfils[56] = FerPun(-16, -27);
	valorposicioalfils[57] = FerPun(-9, -19);
	valorposicioalfils[58] = FerPun(-3, -13);
	valorposicioalfils[59] = FerPun(-12, -9);
	valorposicioalfils[60] = FerPun(-11, -7);
	valorposicioalfils[61] = FerPun(-6, -11);
	valorposicioalfils[62] = FerPun(-5, -14);
	valorposicioalfils[63] = FerPun(-16, -22);

	valorposiciodama[0] = FerPun(-7, -26);
	valorposiciodama[1] = FerPun(-2, -19);
	valorposiciodama[2] = FerPun(2, -13);
	valorposiciodama[3] = FerPun(4, -7);
	valorposiciodama[4] = FerPun(4, -5);
	valorposiciodama[5] = FerPun(-3, -13);
	valorposiciodama[6] = FerPun(-8, -21);
	valorposiciodama[7] = FerPun(-11, -28);
	valorposiciodama[8] = FerPun(-2, -19);
	valorposiciodama[9] = FerPun(-3, -11);
	valorposiciodama[10] = FerPun(5, -7);
	valorposiciodama[11] = FerPun(5, -2);
	valorposiciodama[12] = FerPun(2, -1);
	valorposiciodama[13] = FerPun(4, -7);
	valorposiciodama[14] = FerPun(-2, -11);
	valorposiciodama[15] = FerPun(-2, -20);
	valorposiciodama[16] = FerPun(-3, -13);
	valorposiciodama[17] = FerPun(-2, -6);
	valorposiciodama[18] = FerPun(2, -1);
	valorposiciodama[19] = FerPun(3, 1);
	valorposiciodama[20] = FerPun(3, 1);
	valorposiciodama[21] = FerPun(3, -2);
	valorposiciodama[22] = FerPun(2, -7);
	valorposiciodama[23] = FerPun(-2, -12);
	valorposiciodama[24] = FerPun(3, -7);
	valorposiciodama[25] = FerPun(-2, -2);
	valorposiciodama[26] = FerPun(2, 3);
	valorposiciodama[27] = FerPun(3, 8);
	valorposiciodama[28] = FerPun(1, 9);
	valorposiciodama[29] = FerPun(3, 2);
	valorposiciodama[30] = FerPun(0, -1);
	valorposiciodama[31] = FerPun(-2, -6);
	valorposiciodama[32] = FerPun(0, -7);
	valorposiciodama[33] = FerPun(-1, -2);
	valorposiciodama[34] = FerPun(6, 3);
	valorposiciodama[35] = FerPun(4, 8);
	valorposiciodama[36] = FerPun(4, 8);
	valorposiciodama[37] = FerPun(5, 3);
	valorposiciodama[38] = FerPun(-1, -3);
	valorposiciodama[39] = FerPun(-1, -5);
	valorposiciodama[40] = FerPun(-1, -13);
	valorposiciodama[41] = FerPun(1, -7);
	valorposiciodama[42] = FerPun(5, -2);
	valorposiciodama[43] = FerPun(7, 3);
	valorposiciodama[44] = FerPun(4, 3);
	valorposiciodama[45] = FerPun(5, -1);
	valorposiciodama[46] = FerPun(1, -6);
	valorposiciodama[47] = FerPun(-1, -13);
	valorposiciodama[48] = FerPun(-4, -17);
	valorposiciodama[49] = FerPun(1, -6);
	valorposiciodama[50] = FerPun(3, -2);
	valorposiciodama[51] = FerPun(1, 2);
	valorposiciodama[52] = FerPun(0, 3);
	valorposiciodama[53] = FerPun(1, -3);
	valorposiciodama[54] = FerPun(1, -6);
	valorposiciodama[55] = FerPun(-4, -17);
	valorposiciodama[56] = FerPun(-11, -27);
	valorposiciodama[57] = FerPun(-2, -19);
	valorposiciodama[58] = FerPun(0, -11);
	valorposiciodama[59] = FerPun(0, -6);
	valorposiciodama[60] = FerPun(-2, -7);
	valorposiciodama[61] = FerPun(1, -11);
	valorposiciodama[62] = FerPun(-6, -19);
	valorposiciodama[63] = FerPun(-10, -26);

	valorposiciotorres[0] = FerPun(-7, 2);
	valorposiciotorres[1] = FerPun(-2, 2);
	valorposiciotorres[2] = FerPun(-2, 3);
	valorposiciotorres[3] = FerPun(6, -1);
	valorposiciotorres[4] = FerPun(8, 0);
	valorposiciotorres[5] = FerPun(-2, 3);
	valorposiciotorres[6] = FerPun(-2, 1);
	valorposiciotorres[7] = FerPun(-6, -3);
	valorposiciotorres[8] = FerPun(-10, 3);
	valorposiciotorres[9] = FerPun(-2, 2);
	valorposiciotorres[10] = FerPun(-4, -1);
	valorposiciotorres[11] = FerPun(0, -2);
	valorposiciotorres[12] = FerPun(-1, -2);
	valorposiciotorres[13] = FerPun(-3, 1);
	valorposiciotorres[14] = FerPun(-5, 3);
	valorposiciotorres[15] = FerPun(-9, 0);
	valorposiciotorres[16] = FerPun(-6, 3);
	valorposiciotorres[17] = FerPun(-3, 2);
	valorposiciotorres[18] = FerPun(-2, 2);
	valorposiciotorres[19] = FerPun(1, 1);
	valorposiciotorres[20] = FerPun(0, 4);
	valorposiciotorres[21] = FerPun(-5, 3);
	valorposiciotorres[22] = FerPun(-3, 3);
	valorposiciotorres[23] = FerPun(-10, 3);
	valorposiciotorres[24] = FerPun(-6, 7);
	valorposiciotorres[25] = FerPun(-2, 5);
	valorposiciotorres[26] = FerPun(-2, 7);
	valorposiciotorres[27] = FerPun(2, 7);
	valorposiciotorres[28] = FerPun(0, 7);
	valorposiciotorres[29] = FerPun(0, 6);
	valorposiciotorres[30] = FerPun(-3, 6);
	valorposiciotorres[31] = FerPun(-6, 5);
	valorposiciotorres[32] = FerPun(-4, 7);
	valorposiciotorres[33] = FerPun(0, 8);
	valorposiciotorres[34] = FerPun(0, 10);
	valorposiciotorres[35] = FerPun(2, 10);
	valorposiciotorres[36] = FerPun(2, 8);
	valorposiciotorres[37] = FerPun(0, 8);
	valorposiciotorres[38] = FerPun(0, 8);
	valorposiciotorres[39] = FerPun(-4, 9);
	valorposiciotorres[40] = FerPun(0, 15);
	valorposiciotorres[41] = FerPun(2, 9);
	valorposiciotorres[42] = FerPun(5, 11);
	valorposiciotorres[43] = FerPun(5, 11);
	valorposiciotorres[44] = FerPun(6, 12);
	valorposiciotorres[45] = FerPun(2, 12);
	valorposiciotorres[46] = FerPun(3, 12);
	valorposiciotorres[47] = FerPun(-1, 11);
	valorposiciotorres[48] = FerPun(1, 14);
	valorposiciotorres[49] = FerPun(5, 14);
	valorposiciotorres[50] = FerPun(4, 13);
	valorposiciotorres[51] = FerPun(6, 13);
	valorposiciotorres[52] = FerPun(6, 11);
	valorposiciotorres[53] = FerPun(4, 13);
	valorposiciotorres[54] = FerPun(1, 13);
	valorposiciotorres[55] = FerPun(1, 13);
	valorposiciotorres[56] = FerPun(-4, 6);
	valorposiciotorres[57] = FerPun(0, 7);
	valorposiciotorres[58] = FerPun(0, 7);
	valorposiciotorres[59] = FerPun(2, 9);
	valorposiciotorres[60] = FerPun(3, 7);
	valorposiciotorres[61] = FerPun(1, 8);
	valorposiciotorres[62] = FerPun(0, 7);
	valorposiciotorres[63] = FerPun(-3, 7);

	PenalitzacioAlfilMateixColor1Peo = FerPun(3, 6); //(4, 6)
	PenalitzacioAlfilBloquejatPeoCentral = FerPun(2, 8);
	PenalitzacioAlfilBloquejatPeoCentral2 = FerPun(-1, 3); //tunejat (-1, 3) //no: (0, 3) i (1,3), els dos millor a ràpides
	AlfilClavantCaballvsDama = 4;
	BonusAlfilNoPotSerExpulsatPerPeo[0] = FerPun(1, 0);
	BonusAlfilNoPotSerExpulsatPerPeo[1] = FerPun(-1, 0);
	BonusAlfilNoPotSerExpulsatPerPeo[2] = FerPun(4, 0);
	BonusAlfilNoPotSerExpulsatPerPeo[3] = FerPun(7, 0);
	BonusAlfilNoPotSerExpulsatPerPeo[4] = FerPun(11, 0);
	BonusAlfilNoPotSerExpulsatPerPeo[5] = FerPun(9, 0);
	BonusAlfilNoPotSerExpulsatPerPeo[6] = FerPun(3, 0);
	BonusAlfilNoPotSerExpulsatPerPeo[7] = FerPun(1, 0);
	BonusAlfilSuportatPerPeo = FerPun(5, 9);
	BonusTorreTallaRei7a = FerPun(0, 14);
	BonusTorreColumnaOberta = FerPun(12, 7); //no: (10, 7) (10, 13);  (12, 4) //(15, 7)
	BonusTorreColumnaSemiOberta = FerPun(5, -1); //tunejat (5, -1) //no: (8, 2), (5,1)
	PenalitzacioTorreBloquejada = FerPun(8, -1);
	BonusTorresDoblades = FerPun(22, 7);
	BonusTorreMiraDama = FerPun(12, 9);
	PenalitzacioReiCentreSenseEnroc = 10; //no: 6

	mulpeonsdavantcontrari = 9; //9 //no: 11
	penalitzaciosemibloqueigcentral = 8; //10 //no: 5, 15
	penalitzaciosibloqueigcentral = 13;
	minpenalitzacioreiperdisminuirsiunatacant = 35; //26
	PenalitzacioPeoOcupaDavantDebil = FerPun(5, 4); //5, 9
	PenalitzacioPeoDebil[0] = FerPun(3, 3); //0, 2
	PenalitzacioPeoDebil[1] = FerPun(3, 6); //0, 6
	PenalitzacioPeoDebil[2] = FerPun(11, 6); //11, 6
	PenalitzacioPeoDebil[3] = FerPun(9, 9); //9, 9
	PenalitzacioPeoDebil[4] = FerPun(19, 3); //19, 3
	PenalitzacioPeoDebil[5] = FerPun(3, 3); //0, 0
	PenalitzacioPeoDebil[6] = FerPun(3, 6); //0, 6
	PenalitzacioPeoDebil[7] = FerPun(4, 6); //4, 6
	Aillat[0][0] = FerPun(10, 9);   //(7, 9);   //(13, 9);
	Aillat[0][1] = FerPun(16, 9);  //(14, 9);  //(19, 9);
	Aillat[0][2] = FerPun(21, 18); //(18, 18); //(26, 18);
	Aillat[0][3] = FerPun(21, 19); //(18, 19); //(29, 19);
	Aillat[0][4] = FerPun(21, 16); //(18, 16); //(28, 16);
	Aillat[0][5] = FerPun(17, 15); //(14, 15); //(20, 15);
	Aillat[0][6] = FerPun(15, 15); //(12, 15); //(17, 15);
	Aillat[0][7] = FerPun(16, 19); //(14, 19); //(19, 19);
	Aillat[1][0] = FerPun(5, 10);  //(5, 10);  //(5, 10);
	Aillat[1][1] = FerPun(10, 10);  //(9, 15);  //(12, 15);
	Aillat[1][2] = FerPun(13, 11); //(11, 11); //(16, 11);
	Aillat[1][3] = FerPun(10, 10); //(13, 15); //(10, 15);
	Aillat[1][4] = FerPun(15, 10); //(13, 15); //(19, 15);
	Aillat[1][5] = FerPun(14, 12); //(13, 12); //(17, 12);
	Aillat[1][6] = FerPun(13, 12); //(11, 13); //(16, 13);
	Aillat[1][7] = FerPun(15, 10);  //(9, 10);  //(21, 10);
	PenalitzacioPeoNoSuportat = FerPun(7, 2); //(7, 2) //no: (7, 5)

	memset(Lever, 0, sizeof(Lever));
	Lever[0][3] = FerPun(0, 0);
	Lever[0][4] = FerPun(9, 9);
	Lever[0][5] = FerPun(24, 21); //(24, 21) //no: bo a ràpides: (19, 18)
	Lever[1][4] = FerPun(7, 10); //(7, 10) //no: (7, 7), //no:(10, 10)
	Lever[1][5] = FerPun(16, 13); //(16, 13) //no: (12, 10), (19, 16)

	PenalitzacioPeoDoblatAillat = FerPun(9, 6); //(9, 6) //no:(15, 6), (12, 6)

	PenalitzacioDistanciaReiColumnaPeonsAux[0] = 0;
	PenalitzacioDistanciaReiColumnaPeonsAux[1] = 0;
	PenalitzacioDistanciaReiColumnaPeonsAux[2] = 12;
	PenalitzacioDistanciaReiColumnaPeonsAux[3] = 41;
	PenalitzacioDistanciaReiColumnaPeonsAux[4] = 65;
	PenalitzacioDistanciaReiColumnaPeonsAux[5] = 72;
	PenalitzacioDistanciaReiColumnaPeonsAux[6] = 81;
	PenalitzacioDistanciaReiColumnaPeonsAux[7] = 90;


	CompensacioEntreMaterialIMobilitat1 = 19;
	CompensacioEntreMaterialIMobilitat2 = 22;
	CompensacioEntreMaterialIMobilitat3 = 45;
	ValorSimplificacio2 = 9;
	BonusPerQuiMou = FerPun(10, 4);//no: (6, 4)
	MenorDarreraPeo = 8; //8 //no: 6
	BonusMenysPeonsCentralsSiMesTorresoAlfils[0] = FerPun(6, 10);  //(8, 15) era millor que (4, 8); deixat (6, 10) sense provar
	BonusMenysPeonsCentralsSiMesTorresoAlfils[1] = FerPun(4, 7); //(8, 10)
	BonusMenysPeonsCentralsSiMesTorresoAlfils[2] = FerPun(2, 3);
	BonusMenysPeonsCentralsSiMesTorresoAlfils[3] = FerPun(2, 1);
	BonusMenysPeonsCentralsSiMesTorresoAlfils[4] = FerPun(1, 0);
	BonusMenysPeonsCentralsSiMesTorresoAlfils[5] = FerPun(1, 0);
	BonusMenysPeonsCentralsSiMesTorresoAlfils[6] = FerPun(-2, -2);
	BonusMenysPeonsCentralsSiMesTorresoAlfils[7] = FerPun(-13, -12);
	BonusMenysPeonsCentralsSiMesTorresoAlfils[8] = FerPun(-15, -13);
	BonusAlfilPerPeonsEnDosFlancs = FerPun(20, 5); //bo: (20, 5) //tunejat (26, 5) //no:(15, 5), (27, 1)

	PenalitzacioParellaAlfilsSiMoltsPeons = FerPun(8, 2); //(3, 2)
	BonusParellaAlfilsNoCanviables = FerPun(7, 5);
	PenalitzacioTorreICaballposicioOriginal = FerPun(0, 0);

	PenalitzacioDiferenciaDesenvolupament[0] = 0;
	PenalitzacioDiferenciaDesenvolupament[1] = 0;
	PenalitzacioDiferenciaDesenvolupament[2] = FerPun(5, 8);
	PenalitzacioDiferenciaDesenvolupament[3] = FerPun(14, 12);
	PenalitzacioDiferenciaDesenvolupament[4] = FerPun(25, 24);
	PenalitzacioDiferenciaDesenvolupament[5] = FerPun(42, 42);
	PenalitzacioDiferenciaDesenvolupament[6] = FerPun(106, 107);
	PenalitzacioDiferenciaDesenvolupament[7] = FerPun(164, 165);
	PenalitzacioDiferenciaDesenvolupament[8] = FerPun(235, 235);
	PenalitzacioDiferenciaDesenvolupament[9] = FerPun(234, 234);
	PenalitzacioDiferenciaDesenvolupament[10] = FerPun(251, 251);

	valorposiciopeonspassats[blanques][8] = FerPun(-28, -53); valorposiciopeonspassats[negres][48] = FerPun(-28, -53);  valorposiciopeonspassats[blanques][9] = FerPun(-28, -53); valorposiciopeonspassats[negres][49] = FerPun(-28, -53);  valorposiciopeonspassats[blanques][10] = FerPun(-28, -53); valorposiciopeonspassats[negres][50] = FerPun(-28, -53);  valorposiciopeonspassats[blanques][11] = FerPun(-28, -53); valorposiciopeonspassats[negres][51] = FerPun(-28, -53);  valorposiciopeonspassats[blanques][12] = FerPun(-28, -53); valorposiciopeonspassats[negres][52] = FerPun(-28, -53);  valorposiciopeonspassats[blanques][13] = FerPun(-28, -53); valorposiciopeonspassats[negres][53] = FerPun(-28, -53);  valorposiciopeonspassats[blanques][14] = FerPun(-28, -53); valorposiciopeonspassats[negres][54] = FerPun(-28, -53);  valorposiciopeonspassats[blanques][15] = FerPun(-28, -53); valorposiciopeonspassats[negres][55] = FerPun(-28, -53);
	valorposiciopeonspassats[blanques][16] = FerPun(-27, -29); valorposiciopeonspassats[negres][40] = FerPun(-27, -29);  valorposiciopeonspassats[blanques][17] = FerPun(-27, -29); valorposiciopeonspassats[negres][41] = FerPun(-27, -29);  valorposiciopeonspassats[blanques][18] = FerPun(-27, -29); valorposiciopeonspassats[negres][42] = FerPun(-27, -29);  valorposiciopeonspassats[blanques][19] = FerPun(-27, -29); valorposiciopeonspassats[negres][43] = FerPun(-27, -29);  valorposiciopeonspassats[blanques][20] = FerPun(-27, -29); valorposiciopeonspassats[negres][44] = FerPun(-27, -29);  valorposiciopeonspassats[blanques][21] = FerPun(-27, -29); valorposiciopeonspassats[negres][45] = FerPun(-27, -29);  valorposiciopeonspassats[blanques][22] = FerPun(-27, -29); valorposiciopeonspassats[negres][46] = FerPun(-27, -29);  valorposiciopeonspassats[blanques][23] = FerPun(-27, -29); valorposiciopeonspassats[negres][47] = FerPun(-27, -29);
	valorposiciopeonspassats[blanques][24] = FerPun(-6, 25); valorposiciopeonspassats[negres][32] = FerPun(-6, 25);  valorposiciopeonspassats[blanques][25] = FerPun(-6, 25); valorposiciopeonspassats[negres][33] = FerPun(-6, 25);  valorposiciopeonspassats[blanques][26] = FerPun(-6, 25); valorposiciopeonspassats[negres][34] = FerPun(-6, 25);  valorposiciopeonspassats[blanques][27] = FerPun(-6, 25); valorposiciopeonspassats[negres][35] = FerPun(-6, 25);  valorposiciopeonspassats[blanques][28] = FerPun(-6, 25); valorposiciopeonspassats[negres][36] = FerPun(-6, 25);  valorposiciopeonspassats[blanques][29] = FerPun(-6, 25); valorposiciopeonspassats[negres][37] = FerPun(-6, 25);  valorposiciopeonspassats[blanques][30] = FerPun(-6, 25); valorposiciopeonspassats[negres][38] = FerPun(-6, 25);  valorposiciopeonspassats[blanques][31] = FerPun(-6, 25); valorposiciopeonspassats[negres][39] = FerPun(-6, 25);
	valorposiciopeonspassats[blanques][32] = FerPun(35, 71); valorposiciopeonspassats[negres][24] = FerPun(35, 71);  valorposiciopeonspassats[blanques][33] = FerPun(35, 71); valorposiciopeonspassats[negres][25] = FerPun(35, 71);  valorposiciopeonspassats[blanques][34] = FerPun(35, 71); valorposiciopeonspassats[negres][26] = FerPun(35, 71);  valorposiciopeonspassats[blanques][35] = FerPun(35, 71); valorposiciopeonspassats[negres][27] = FerPun(35, 71);  valorposiciopeonspassats[blanques][36] = FerPun(35, 71); valorposiciopeonspassats[negres][28] = FerPun(35, 71);  valorposiciopeonspassats[blanques][37] = FerPun(35, 71); valorposiciopeonspassats[negres][29] = FerPun(35, 71);  valorposiciopeonspassats[blanques][38] = FerPun(35, 71); valorposiciopeonspassats[negres][30] = FerPun(35, 71);  valorposiciopeonspassats[blanques][39] = FerPun(35, 71); valorposiciopeonspassats[negres][31] = FerPun(35, 71);
	valorposiciopeonspassats[blanques][40] = FerPun(83, 145); valorposiciopeonspassats[negres][16] = FerPun(83, 145);  valorposiciopeonspassats[blanques][41] = FerPun(83, 145); valorposiciopeonspassats[negres][17] = FerPun(83, 145);  valorposiciopeonspassats[blanques][42] = FerPun(83, 145); valorposiciopeonspassats[negres][18] = FerPun(83, 145);  valorposiciopeonspassats[blanques][43] = FerPun(83, 145); valorposiciopeonspassats[negres][19] = FerPun(83, 145);  valorposiciopeonspassats[blanques][44] = FerPun(83, 145); valorposiciopeonspassats[negres][20] = FerPun(83, 145);  valorposiciopeonspassats[blanques][45] = FerPun(83, 145); valorposiciopeonspassats[negres][21] = FerPun(83, 145);  valorposiciopeonspassats[blanques][46] = FerPun(83, 145); valorposiciopeonspassats[negres][22] = FerPun(83, 145);  valorposiciopeonspassats[blanques][47] = FerPun(83, 145); valorposiciopeonspassats[negres][23] = FerPun(83, 145);
	valorposiciopeonspassats[blanques][48] = FerPun(128, 228); valorposiciopeonspassats[negres][8] = FerPun(128, 228);  valorposiciopeonspassats[blanques][49] = FerPun(128, 228); valorposiciopeonspassats[negres][9] = FerPun(128, 228);  valorposiciopeonspassats[blanques][50] = FerPun(128, 228); valorposiciopeonspassats[negres][10] = FerPun(128, 228);  valorposiciopeonspassats[blanques][51] = FerPun(128, 228); valorposiciopeonspassats[negres][11] = FerPun(128, 228);  valorposiciopeonspassats[blanques][52] = FerPun(128, 228); valorposiciopeonspassats[negres][12] = FerPun(128, 228);  valorposiciopeonspassats[blanques][53] = FerPun(128, 228); valorposiciopeonspassats[negres][13] = FerPun(128, 228);  valorposiciopeonspassats[blanques][54] = FerPun(128, 228); valorposiciopeonspassats[negres][14] = FerPun(128, 228);  valorposiciopeonspassats[blanques][55] = FerPun(128, 228); valorposiciopeonspassats[negres][15] = FerPun(128, 228);
	penalitzacio_rei_contrari_proper_peons_passats_final[1] = FerPun(0, 67);
	penalitzacio_rei_contrari_proper_peons_passats_final[2] = FerPun(0, 21);
	penalitzacio_rei_contrari_proper_peons_passats_final[3] = FerPun(0, 6);
	penalitzacio_rei_contrari_proper_peons_passats_final[4] = FerPun(0, -7);
	penalitzacio_rei_contrari_proper_peons_passats_final[5] = FerPun(0, -17);
	penalitzacio_rei_contrari_proper_peons_passats_final[6] = FerPun(0, -21);
	penalitzacio_rei_contrari_proper_peons_passats_final[7] = FerPun(0, -26);
	bonificacio_rei_proper_peons_passats_final[1] = FerPun(0, 21);
	bonificacio_rei_proper_peons_passats_final[2] = FerPun(0, 10);
	bonificacio_rei_proper_peons_passats_final[3] = FerPun(0, -13);
	bonificacio_rei_proper_peons_passats_final[4] = FerPun(0, -17);
	bonificacio_rei_proper_peons_passats_final[5] = FerPun(0, -12);
	bonificacio_rei_proper_peons_passats_final[6] = FerPun(0, -12);
	bonificacio_rei_proper_peons_passats_final[7] = FerPun(0, -36);
	BonificacioPassatAmbSuport = FerPun(1, 16); //(1, 16)  //1,10 és bastant pitjor que 1,20
	FilaPassat[0] = FerPun(-2, 15);
	FilaPassat[1] = FerPun(2, 12);
	FilaPassat[2] = FerPun(-8, 4);
	FilaPassat[3] = FerPun(-9, 0);
	FilaPassat[4] = FerPun(-6, 2);
	FilaPassat[5] = FerPun(6, 7);
	FilaPassat[6] = FerPun(4, 8);
	FilaPassat[7] = FerPun(8, 9);
	bonificacio_base_peons_passats_potser_no_parables = 23;

	CompensacioEntreMaterialIMobilitat1 = 23; //20
	CompensacioEntreMaterialIMobilitat2 = 23;
	CompensacioEntreMaterialIMobilitat3 = 45;
	CompensacioEntreMaterialIMobilitat1Final = 19;
	CompensacioEntreMaterialIMobilitat2Final = 22;
	CompensacioEntreMaterialIMobilitat3Final = 45;

	alfil_per_peons[0] = FerPun(1, 0);
	alfil_per_peons[1] = FerPun(-22, -19);
	alfil_per_peons[2] = FerPun(-11, -8);
	alfil_per_peons[3] = FerPun(-5, -2);
	alfil_per_peons[4] = FerPun(5, 10);
	alfil_per_peons[5] = FerPun(-1, -2);
	caball_per_peons[0] = FerPun(-34, -25);
	caball_per_peons[1] = FerPun(-23, -25);
	caball_per_peons[2] = FerPun(-20, -20); //(-1, 0)
	caball_per_peons[3] = FerPun(-20, -20); //(-6, 2)
	caball_per_peons[4] = FerPun(-20, -20); //(6, 1)
	caball_per_peons[5] = FerPun(-20, -20); //(-1, -2)

	caball_i_peons_per_torre[0] = FerPun(-15, 5); //(-15, 5) //no (-15, 12)
	caball_i_peons_per_torre[1] = FerPun(-10, -4); //(-10, -4) //no (-10, 4)
	caball_i_peons_per_torre[2] = FerPun(-5, -4); //(-5, -4) //no (-5, 4)
	caball_i_peons_per_torre[3] = FerPun(5, 4); //(5, 4)
	caball_i_peons_per_torre[4] = FerPun(3, 8);
	caball_i_peons_per_torre[5] = FerPun(-2, 1);

	alfil_i_peons_per_torre[0] = FerPun(-15, -5);
	alfil_i_peons_per_torre[1] = FerPun(-10, 0);
	alfil_i_peons_per_torre[2] = FerPun(-5, -15);
	alfil_i_peons_per_torre[3] = FerPun(-5, -15);
	alfil_i_peons_per_torre[4] = FerPun(-5, -15);
	alfil_i_peons_per_torre[5] = FerPun(2, 5);
	alfil_i_caball_per_torre_i_peons[0] = FerPun(0, -36);
	alfil_i_caball_per_torre_i_peons[1] = FerPun(0, -25);
	alfil_i_caball_per_torre_i_peons[2] = FerPun(0, -22);
	alfil_i_caball_per_torre_i_peons[3] = FerPun(0, -6);
	alfil_i_caball_per_torre_i_peons[4] = FerPun(0, -23);
	alfil_i_caball_per_torre_i_peons[5] = FerPun(0, -30);
	dama_per_torre_caball_i_peons[0] = FerPun(-24, -33);
	dama_per_torre_caball_i_peons[1] = FerPun(-34, -34);
	dama_per_torre_caball_i_peons[2] = FerPun(-19, -23);
	dama_per_torre_caball_i_peons[3] = FerPun(-5, -6);
	dama_per_torre_caball_i_peons[4] = FerPun(-1, -1);
	dama_per_torre_caball_i_peons[5] = FerPun(0, 0);
	dama_per_torre_alfil_i_peons[0] = FerPun(-16, -21);
	dama_per_torre_alfil_i_peons[1] = FerPun(-17, -23);
	dama_per_torre_alfil_i_peons[2] = FerPun(-7, -15);
	dama_per_torre_alfil_i_peons[3] = FerPun(-5, -5);
	dama_per_torre_alfil_i_peons[4] = FerPun(-2, -5);
	dama_per_torre_alfil_i_peons[5] = FerPun(0, 0);
	dama_per_torre_alfil_amb_parella_i_peons[0] = FerPun(-17, -20);
	dama_per_torre_alfil_amb_parella_i_peons[1] = FerPun(-26, -28);
	dama_per_torre_alfil_amb_parella_i_peons[2] = FerPun(-7, -12);
	dama_per_torre_alfil_amb_parella_i_peons[3] = FerPun(-5, -5);
	dama_per_torre_alfil_amb_parella_i_peons[4] = FerPun(-1, -4);
	dama_per_torre_alfil_amb_parella_i_peons[5] = FerPun(0, 0);

	torre_i_peons_per_dos_alfils[0] = FerPun(25, 25);
	torre_i_peons_per_dos_alfils[1] = FerPun(25, 25);
	torre_i_peons_per_dos_alfils[2] = FerPun(25, 25);
	torre_i_peons_per_dos_alfils[3] = FerPun(25, 25);
	torre_i_peons_per_dos_alfils[4] = FerPun(25, 25);
	torre_i_peons_per_dos_alfils[5] = FerPun(25, 25);

	dama_i_peons_per_torres[0] = FerPun(20, -10);
	dama_i_peons_per_torres[1] = FerPun(20, -10);
	dama_i_peons_per_torres[2] = FerPun(20, -10);
	dama_i_peons_per_torres[3] = FerPun(20, -10);
	dama_i_peons_per_torres[4] = FerPun(20, -10);
	dama_i_peons_per_torres[5] = FerPun(20, -10);

	PenalitzacioCaballBloquejaPeoC = FerPun(4, 0);

	mobilitatc[0] = FerPun(-49, -37);
	mobilitatc[1] = FerPun(-26, -18);
	mobilitatc[2] = FerPun(-14, -13);
	mobilitatc[3] = FerPun(-6, -5);
	mobilitatc[4] = FerPun(-4, 2);
	mobilitatc[5] = FerPun(4, 7);
	mobilitatc[6] = FerPun(6, 8);
	mobilitatc[7] = FerPun(12, 9);
	mobilitatc[8] = FerPun(18, 11);

	MinValorCaballsPerBonusvsAlfilsBons = 25;
	BonusParellaAlfilsvsCaballsBons = 24; //no: 29;
	BonusParellaAlfilsvsCaballsBonsFinal = FerPun(0, 37); //(0, 37) //no (0, 32)
	BonusParellaAlfils = 40; //40  //no: 35 un pel pitjor
	BonusParellaAlfilsFinal = FerPun(0, 45); //bo (0, 45) //no: (0, 40), (0, 43), (0, 52);

	mobilitata[0] = FerPun(-30, -56);
	mobilitata[1] = FerPun(-21, -30); //(-21, -30) //no(-24, -30)
	mobilitata[2] = FerPun(-10, -20); //(-10, -20) //no:(-13, -20) (-16, -20)
	mobilitata[3] = FerPun(-7, -8); //(-7, -8)//no:(-9, -8)
	mobilitata[4] = FerPun(2, 0); //(2, 0) //no:(0, 0) (-2, 0)
	mobilitata[5] = FerPun(5, 10); //(5, 10) //no:(7, 10)
	mobilitata[6] = FerPun(8, 10);
	mobilitata[7] = FerPun(10, 13);
	mobilitata[8] = FerPun(14, 16);
	mobilitata[9] = FerPun(18, 16); //(18, 16)//no:(15, 16)
	mobilitata[10] = FerPun(23, 17);
	mobilitata[11] = FerPun(26, 22);
	mobilitata[12] = FerPun(29, 25);
	mobilitata[13] = FerPun(30, 26);

	mobilitatt[0] = FerPun(-33, -50);
	mobilitatt[1] = FerPun(-19, -26);
	mobilitatt[2] = FerPun(-15, -20);
	mobilitatt[3] = FerPun(-11, -14);
	mobilitatt[4] = FerPun(-9, -12);
	mobilitatt[5] = FerPun(-7, 0);
	mobilitatt[6] = FerPun(-5, 7);
	mobilitatt[7] = FerPun(2, 15);
	mobilitatt[8] = FerPun(5, 22);
	mobilitatt[9] = FerPun(8, 27);
	mobilitatt[10] = FerPun(9, 34);
	mobilitatt[11] = FerPun(17, 39);
	mobilitatt[12] = FerPun(19, 42);
	mobilitatt[13] = FerPun(20, 43);
	mobilitatt[14] = FerPun(20, 46);

	mobilitatd[0] = FerPun(-42, -43);
	mobilitatd[1] = FerPun(-34, -43);
	mobilitatd[2] = FerPun(-30, -35);
	mobilitatd[3] = FerPun(-24, -34);
	mobilitatd[4] = FerPun(-13, -20);
	mobilitatd[5] = FerPun(-13, -17);
	mobilitatd[6] = FerPun(-9, -11);
	mobilitatd[7] = FerPun(-5, -5);
	mobilitatd[8] = FerPun(-2, -3);
	mobilitatd[9] = FerPun(-1, 5);
	mobilitatd[10] = FerPun(2, 6);
	mobilitatd[11] = FerPun(5, 9);
	mobilitatd[12] = FerPun(7, 9);
	mobilitatd[13] = FerPun(7, 12);
	mobilitatd[14] = FerPun(8, 12);
	mobilitatd[15] = FerPun(9, 13);
	mobilitatd[16] = FerPun(10, 13);
	mobilitatd[17] = FerPun(10, 13);
	mobilitatd[18] = FerPun(10, 13);
	mobilitatd[19] = FerPun(11, 13);
	mobilitatd[20] = FerPun(11, 13);
	mobilitatd[21] = FerPun(11, 13);
	mobilitatd[22] = FerPun(12, 13);
	mobilitatd[23] = FerPun(12, 13);
	mobilitatd[24] = FerPun(12, 13);
	mobilitatd[25] = FerPun(13, 13);
	mobilitatd[26] = FerPun(13, 13);
	mobilitatd[27] = FerPun(13, 13);

	BonusCaballNoPotSerExpulsatPerPeo[0] = FerPun(0, 4);
	BonusCaballNoPotSerExpulsatPerPeo[1] = FerPun(4, 3);
	BonusCaballNoPotSerExpulsatPerPeo[2] = FerPun(3, 1);
	BonusCaballNoPotSerExpulsatPerPeo[3] = FerPun(1, 6);
	BonusCaballNoPotSerExpulsatPerPeo[4] = FerPun(18, 8);
	BonusCaballNoPotSerExpulsatPerPeo[5] = FerPun(9, 5);
	BonusCaballNoPotSerExpulsatPerPeo[6] = FerPun(11, 10);
	BonusCaballNoPotSerExpulsatPerPeo[7] = FerPun(10, 4); //(20, 4)
	BonusCaballSuportatPerPeo = FerPun(-1, -3);
	PenalitzacioCaballAtrapatPeons = FerPun(1, 1);
}

template<e_colors jo>
int avaluacio_caballs(tss * RESTRICT ss, struct InfoEval *ie) {
	int i, mpun, pun, j, c, cc;
	const e_colors opo = (jo == blanques ? negres : blanques);
	pun = 0;
	Bitboard atacscaball, atacsrei = 0, a, kk, ll;
	int avant;
	int darreres3files = jo ? 1 + 2 + 4 : 32 + 64 + 128;

	i = 0;
	Estat_Analisis *estat_actual = ss->estat_actual;
	int penpeons = (NumPeons(blanques) + NumPeons(negres)) - 8;
	for (a = Tcaballs(jo); a != 0; a &= a - 1, i++) {
		avant = pun;
		c = lsb(a);

		//caball perd valor com menys peons hi ha
		pun += penpeons;

		if (jo == blanques)
			j = vp_caballs[c];
		else
			j = vp_caballs[63 - c];

		atacscaball = AtacsCaball[c];
		//mobilitat. Atacs de la peça: treure atacs peons. 
		Bitboard mobcaball = atacscaball & (~ie->atacspeons_per_mobilitat[jo]);

		Bitboard bb = ~ie->thp->pawnAttacksSpan[opo];
		if (bb & Bit(c)) { //si no pot ser atacat per un peo rival
			pun += j;
			j = BonusCaballNoPotSerExpulsatPerPeo[COL(c)];
			pun += j;
			//si el caball no pot ser canviat, incrementar bonus
			if (!Tcaballs(opo) && !(quadresdecolor(Bit(c)) & Talfils(opo))) {
				int pun2 = pun - avant;
				pun += FerPun(PunI(pun2) >> 2, PunF(pun2) >> 2) + FerPun(PunI(pun2) >> 4, PunF(pun2) >> 4); //20
			}
			if (Distancia(c, ie->rei[opo]) <= 3 && (~(estat_actual->atacscaballs[opo] | estat_actual->atacsalfils[opo]) & Bit(c))) //outpost proper al rei, i que no pot ser canviat
				pun += FerPun(5, 0);
		}
		else {
			if (PunI(j) >= 0) //si pot ser expulsat per peó, el valor posició és la meitat (si no negatiu)
				pun += j - FerPun(PunI(j) >> 1, PunF(j) >> 1);
			else
				pun += j;
			bb &= mobcaball & ~Tpeces(jo);
			if (bb) { //reachableoutpost
				if (estat_actual->atacspeons[jo] & bb)
					pun += FerPun(11, 3);
				else  {
					bb = bb & ~(estat_actual->atacscaballs[opo] | estat_actual->atacsalfils[opo]);
					if (bb)
						pun += FerPun(7, 2);
				}
			}
		}

		j = popcount(mobcaball); //no es pot memoritzar dins memoria_caballs

		if (ss->estat_actual->atacspeons[jo] & Bit(c))
			pun += BonusCaballSuportatPerPeo;

		if (ie->claven[opo] & Bit(c)) {
			if (j > 0)
				j--; //si caball està clavat, menys mobilitat
			if (!(estat_actual->atacspeons[jo] & Bit(c)) && !(CasellesDavant[jo][c] & (Tpeons(jo) | Tpeons(opo)))) //si no té peons davant
				pun -= FerPun(10, 0); //clavat en una columna oberta sense defensar per peó propi
		}

		if (j < 0)
			j = 0;
		pun += mobilitatc[j];
		ie->vmobilitat[jo] += mobilitatc[j];

		//caball val menys si no pot anar a flanc contrari i està al flanc propi
		if (fila_relativa(jo, ROW(c)) < 4 && !(mobcaball & (jo == blanques ? FILA5 + FILA6 + FILA7 + FILA8 : FILA1 + FILA2 + FILA3 + FILA4)))
			if (PunI(pun - avant) > 0)
				pun -= FerPun(PunI(pun - avant) >> 3, 0);

		//seguretat del rei contrari
		if (Distancia(c, ie->rei[opo]) < 5) {
			if (!atacsrei)
				atacsrei = (ie->bitboard_casellesrei[opo] | AtacsCaball[ie->rei[opo]]) & ~(estat_actual->atacspeons[opo] | Tpeons(jo));

			bool atac = false;
			Bitboard atacsrei2 = atacscaball & atacsrei;
			if (atacsrei2) {
				j = popcount(atacsrei2);
				ie->pes_total_atacs[opo] += PesAtacCaball * j;
				j = popcount(atacsrei2 - (ie->atacs_color_amb_xray[opo] & atacsrei2));  //treure les caselles defensades
				ie->num_total_atacs[opo] += j;
				atac = true;
			}
			atacsrei2 = atacscaball & ie->bitboard_casellesrei[opo];
			if (atacsrei2) {
				ie->atacs_costat[opo] += popcount(atacsrei2)*PesAtacCaball;
				atac = true;
			}
			ie->num_peces_ataquen[opo] += atac;
		}
		atacscaball = atacscaball & (~Tpeces(jo)); //treure caselles on hi ha peces pròpies
		if ((atacscaball & ss->estat_actual->atacspeons[opo]) == atacscaball)
			pun -= PenalitzacioCaballAtrapatPeons;
		if (c == (jo ? cC6 : cC3) && (Tpeons(jo) & (jo ? C7 : C2)))
			pun -= PenalitzacioCaballBloquejaPeoC;

		//caball val menys si pot ser canviat per menor immediatament
		if ((estat_actual->atacscaballs[opo] | estat_actual->atacsalfils[opo]) & Bit(c)) {
			int jj = pun - avant;
			if (PunI(jj) > 0)
				pun -= PunI(jj) >> 2;
			if (PunF(jj) > 0)
				pun -= FerPun(PunF(jj) >> 2, PunF(jj) >> 2);
		}
	}
	ie->ValorCaballs[jo] = pun;
	return pun;
}

template<e_colors jo>
int avaluacio_alfils(tss * RESTRICT ss, struct InfoEval *ie) {
	Estat_Analisis *estat_actual = ss->estat_actual;
	int i, j, pun, c, peonsennegre, peonsenblanc;
	const e_colors opo = (jo == blanques ? negres : blanques);
	Bitboard atacsrei2, a;
	int avant;

	pun = 0;
	i = 0;
	peonsennegre = popcount(Tpeons(jo) & QuadresNegres);
	peonsenblanc = NumPeons(jo) - peonsennegre;
	a = Talfils(jo);
	for (; a != 0; a &= a - 1, i++) {
		avant = pun;
		c = lsb(a);

		bool DarreraPeonsCentralsMobils = false;
		if (color_casella(c) == blanques) {
			pun -= peonsenblanc * PenalitzacioAlfilMateixColor1Peo;
			if (jo == blanques) {
				//bloqueig peo e4
				if (Tpeons(blanques) & E4) {
					if ((Tpeces(negres) & E5) | (Tpeons(blanques) & E5))
						pun -= PenalitzacioAlfilBloquejatPeoCentral;
					else
						DarreraPeonsCentralsMobils = true;
				}
				//el bloqueig del peo a c4 és menys important
				if (Tpeons(blanques) & C4) {
					if ((Tpeces(negres) & C5) | (Tpeons(blanques) & C5))
						pun -= PenalitzacioAlfilBloquejatPeoCentral2;
					else
						DarreraPeonsCentralsMobils = true;
				}
				//Peo central contrari defensat
				if ((Tpeons(negres) & D5) && (Tpeons(negres) & (C6 | E6)))
					pun -= FerPun(3, 3);
			}
			else {
				//bloqueig peo d5
				if (Tpeons(negres) & D5) {
					if ((Tpeces(blanques) & D4) | (Tpeons(negres) & D4))
						pun -= PenalitzacioAlfilBloquejatPeoCentral;
					else
						DarreraPeonsCentralsMobils = true;
				}
				//el bloqueig del peo a f5 és menys important
				if (Tpeons(negres) & F5) {
					if ((Tpeces(blanques) & F4) | (Tpeons(negres) & F4))
						pun -= PenalitzacioAlfilBloquejatPeoCentral2;
					else
						DarreraPeonsCentralsMobils = true;
				}
				//Peo central contrari defensat
				if ((Tpeons(blanques) & E4) && (Tpeons(blanques) & (D3 | F3)))
					pun -= FerPun(3, 3);
			}
		}
		else {
			pun -= peonsennegre * PenalitzacioAlfilMateixColor1Peo;
			if (jo == blanques) {
				//bloqueig peo d4
				if (Tpeons(blanques) & D4) {
					if ((Tpeces(negres) & D5) | (Tpeons(blanques) & D5))
						pun -= PenalitzacioAlfilBloquejatPeoCentral;
					else
						DarreraPeonsCentralsMobils = true;
				}
				//el bloqueig del peo a f4 és menys important
				if (Tpeons(blanques) & F4) {
					if ((Tpeces(negres) & F5) | (Tpeons(blanques) & F5))
						pun -= PenalitzacioAlfilBloquejatPeoCentral2;
					else
						DarreraPeonsCentralsMobils = true;
				}
				//Peo central contrari defensat
				if ((Tpeons(negres) & E5) && (Tpeons(negres) & (D6 | F6)))
					pun -= FerPun(3, 3);
			}
			else {
				//bloqueig peo e5
				if (Tpeons(negres) & E5) {
					if ((Tpeces(blanques) & E4) | (Tpeons(negres) & E4))
						pun -= PenalitzacioAlfilBloquejatPeoCentral;
					else
						DarreraPeonsCentralsMobils = true;
				}
				//el bloqueig del peo a c5 és menys important
				if (Tpeons(negres) & C5) {
					if ((Tpeces(blanques) & C4) | (Tpeons(negres) & C4))
						pun -= PenalitzacioAlfilBloquejatPeoCentral2;
					else
						DarreraPeonsCentralsMobils = true;
				}
				//Peo central contrari defensat
				if ((Tpeons(blanques) & D4) && (Tpeons(blanques) & (E3 | C3)))
					pun -= FerPun(3, 3);
			}
		}

		if (jo == blanques) {
			pun += valorposicioalfils[63 - c];
			//àlfil clavant caball contra dama, al mateix flanc on és el rei, bonificació
			if ((c == cG5 || c == cH4) && ss->tau.c[cF6] == CaballNegre
				&& ((ss->tau.c[cD8] == DamaNegre && ss->tau.c[cE7] == CasellaBuida) || ss->tau.c[cE7] == DamaNegre)
				&& COL(ie->rei[negres]) >= 4) {
				pun += AlfilClavantCaballvsDama;
			}
		}
		else {
			pun += valorposicioalfils[c];
			//àlfil clavant caball contra dama, al mateix flanc on és el rei, bonificació
			if ((c == cG4 || c == cH5) && ss->tau.c[cF3] == CaballBlanc
				&& ((ss->tau.c[cD1] == DamaBlanca && ss->tau.c[cE2] == CasellaBuida) || ss->tau.c[cE2] == DamaBlanca)
				&& COL(ie->rei[blanques]) >= 4) {
				pun += AlfilClavantCaballvsDama;
			}
		}

		if (!(ie->thp->pawnAttacksSpan[opo] & Bit(c))) //si no pot ser atacat per un peo contrari
			pun += BonusAlfilNoPotSerExpulsatPerPeo[COL(c)];

		if (ss->estat_actual->atacspeons[jo] & Bit(c))
			pun += BonusAlfilSuportatPerPeo;

		Bitboard atacs = estat_actual->atacsalfil[jo][i];

		//mobilitat  //de la dama es calcula a torres
		//atacs de la peça. treure atacs peons. 
		atacsrei2 = atacs & (~ie->atacspeons_per_mobilitat[jo]); //** ho feia diferent. no cal 
		//número de caselles atacades restants
		j = popcount(atacsrei2);
		bool clavat = false;
		//si peça està clavada, menys mobilitat
		if (j > 0 && (ie->claven[opo] & Bit(c))) {
			j--;
			clavat = true;
		}
		if (DarreraPeonsCentralsMobils) //alfil no és tant dolent si darrera peons centrals mobils
			j++;
		if (j < 0)
			j = 0;
		if (j > 13)
			j = 13;
		pun += mobilitata[j];
		ie->vmobilitat[jo] += mobilitata[j];

		//alfil val menys si no pot anar a flanc contrari
		if (fila_relativa(jo, ROW(c)) < 4 && !(atacsrei2 & (jo == blanques ? FILA5 + FILA6 + FILA7 + FILA8 : FILA1 + FILA2 + FILA3 + FILA4)))
			if (PunI(pun - avant) > 0)
				pun -= FerPun(PunI(pun - avant) >> 3, 0);

		if (clavat)
			continue;

		//seguretat del rei contrari
		atacsrei2 = atacs & ie->bitboard_atacsrei[opo];
		bool atac = false;
		if (atacsrei2) {
			j = popcount(atacsrei2);
			ie->pes_total_atacs[opo] += PesAtacAlfil * j;
			j = popcount(atacsrei2 & (~ie->atacs_color_amb_xray[opo]));  //treure les caselles defensades
			ie->num_total_atacs[opo] += j;
			atac = true;
		}
		else if (PrecalculAlfil[ie->rei[opo]] & Bit(c))
			//atacant per la diagonal
			ie->num_total_atacs[opo] += !varis_bits((Tpeons(blanques) | Tpeons(negres)) & Entre[ie->rei[opo]][c]);

		Bitboard atacsxray = atacs;
		if (atacs & Tdames(jo)) {
			atacsxray = AtacsAlfil(c, TTotespeces ^ Tdames(jo));
			ie->atacs_color_amb_xray[jo] |= atacsxray;
		}

		atacsrei2 = atacsxray & ie->bitboard_casellesrei[opo]; //atacs
		if (atacsrei2) {
			ie->atacs_costat[opo] += popcount(atacsrei2)*PesAtacAlfil;
			atac = true;
		}
		ie->num_peces_ataquen[opo] += atac;
	}
	return pun;
}

template<e_colors jo>
int avaluacio_dames(tss * RESTRICT ss, struct InfoEval *ie) {
	Estat_Analisis *estat_actual = ss->estat_actual;
	int i, j, c, fila, columna, punt = 0, punt2;
	const e_colors opo = (jo == blanques ? negres : blanques);
	Bitboard atacsrei2, a, atacs;
	int fila7 = jo ? 1 : 6;
	int fila8 = jo ? 0 : 7;

	i = 0;
	a = Tdames(jo);
	for (; a != 0; a &= a - 1, i++) {
		c = lsb(a);
		columna = COL(c);
		fila = ROW(c);

		if (fila == fila7) {
			if (ROW(ie->rei[opo]) == fila8) {
				punt += BonusTorreTallaRei7a; //només afecta al final
			}
		}
		if (jo == blanques)
			punt += valorposiciodama[c];
		else
			punt += valorposiciodama[63 - c];

		atacs = estat_actual->atacsdama[jo][i];

		//mobilitat
		//atacs de la peça. treure atacs peces menors valor
		Bitboard bb = ie->atacspeons_per_mobilitat[jo]
			| estat_actual->atacscaballs[opo]
			| estat_actual->atacsalfils[opo]
			| estat_actual->atacstorres[opo];
		atacsrei2 = atacs & (~bb); //** ho feia diferent. no cal 
		//número de caselles atacades restants
		j = popcount(atacsrei2);
		//si peça està clavada, menys mobilitat
		if (j > 0 && (ie->claven[opo] & Bit(c)))
			j--;
		if (j < 0)
			j = 0;
		punt += mobilitatd[j];
		ie->vmobilitat[jo] += mobilitatd[j];

		//penalització dama al flanc contrari del rei contrari i no pot tornar
		if (columna < 3 && COL(ie->rei[opo]) > 4 && (atacsrei2 & (COLUMNAA + COLUMNAB + COLUMNAC)) == atacsrei2)
			punt -= FerPun(5, 0);

		bool oberta = false;
		Bitboard peopropidavant = (CasellesDavant[jo][c] & Tpeons(jo));
		bool atac = false;
		if (!peopropidavant) {  //si la torre no té peons propis davant  //** en comptes de CasellesDavant, aquí i a altres llocs, es pot usar un CasellesDavant de només una fila, i fer shift per la columna
			punt2 = 0;
			if (!(Tpeons(opo) & Columnes[columna])) {
				oberta = true;
			}
			j = abs(columna - COL(ie->rei[opo]));
			if (j < 2 || (j == 2 && oberta)) {
				ie->atacs_columnes[opo]++;
				atac = true;
			}
			else
				if (abs(fila - ROW(ie->rei[opo])) < 2 &&
					!varis_bits((Tpeons(blanques) | Tpeons(negres)) & Entre[ie->rei[opo]][c])
					) {
					atac = true;
				}
			punt += punt2;
		}
		else {
			if (abs(columna - COL(ie->rei[opo])) < 2 && fila_relativa(jo, ROW(lsb(peopropidavant))) > 3 && (estat_actual->atacspeons[opo] & peopropidavant))
				atac = true; //està obrint columna sobre el rei
		}

		//seguretat del rei contrari
		atacsrei2 = atacs & ie->bitboard_atacsrei[opo];
		if (atacsrei2) {
			j = popcount(atacsrei2);
			ie->pes_total_atacs[opo] += PesAtacDama * j;
			j = popcount(atacsrei2 & (~ie->atacs_color_amb_xray[opo]));  //treure les caselles defensades
			ie->num_total_atacs[opo] += j;
			atac = true;
			//num_peces_ataquen_negres++;
		}
		else if ((PrecalculAlfil[ie->rei[opo]] | PrecalculTorre[ie->rei[opo]]) & Bit(c))
			//atacant a través de diagonal
			ie->num_total_atacs[opo] += !varis_bits((Tpeons(blanques) | Tpeons(negres)) & Entre[ie->rei[opo]][c]);
		atacsrei2 = atacs & ie->bitboard_casellesrei[opo];
		if (atacsrei2) {
			ie->atacs_costat[opo] += popcount(atacsrei2)*PesAtacDama;
			atac = true;
		}
		if (!atac) {
			atacsrei2 = PrecalculAlfil[c] & ie->bitboard_atacsrei[opo];
			if (atacsrei2) {
				if (!((Tpeons(blanques) | Tpeons(negres)) & Entre[lsb(atacsrei2)][c]))
					atac = true; //ataca per la diagonal
			}
		}
		ie->ataquen_dames[opo] += atac;
		ie->num_peces_ataquen[opo] += atac;
	}
	return punt;
}

template<e_colors jo>
int avaluacio_torres(tss * RESTRICT ss, struct InfoEval *ie) {
	Estat_Analisis *estat_actual = ss->estat_actual;
	int punt = 0, punt2, c, fila, columna, colant = -1, j;
	const e_colors opo = (jo == blanques ? negres : blanques);
	Bitboard a, atacs, atacsrei2;
	int fila7 = jo ? 1 : 6;
	int fila8 = jo ? 0 : 7;

	//penalització per torres bloquejades pel rei
	if (jo == blanques) {
		if ((Trei(blanques) & (E1 + F1 + G1)) && (Ttorres(blanques) & (G1 + H1))) {
			ie->DiferenciaDesenvolupament--;
			punt -= PenalitzacioTorreBloquejada;
			if (!(estat_actual->enroc & PotEnrocarBlancCurt)) {
				ie->DiferenciaDesenvolupament--;
			}
		}
		else if ((Trei(blanques) & (B1 + C1 + D1)) && (Ttorres(blanques) & (A1 + B1))) {
			ie->DiferenciaDesenvolupament--;
			punt -= PenalitzacioTorreBloquejada;
			if (!(estat_actual->enroc & PotEnrocarBlancLlarg)) {
				ie->DiferenciaDesenvolupament--;
			}
		}

	}
	else {
		if ((Trei(negres) & (E8 + F8 + G8)) && (Ttorres(negres) & (G8 + H8))) {
			ie->DiferenciaDesenvolupament++;
			punt -= PenalitzacioTorreBloquejada;
			if (!(estat_actual->enroc & (PotEnrocarNegreCurt + PotEnrocarNegreLlarg))) {
				ie->DiferenciaDesenvolupament++;
			}
		}
		else if ((Trei(negres) & (B8 + C8 + D8)) && (Ttorres(negres) & (A8 + B8))) {
			ie->DiferenciaDesenvolupament++;
			punt -= PenalitzacioTorreBloquejada;
			if (!(estat_actual->enroc & PotEnrocarNegreLlarg)) {
				ie->DiferenciaDesenvolupament++;
			}
		}
	}
	a = Ttorres(jo);
	int filaant = -1;
	int i = 0;
	int guatorres = (8 - (NumPeons(blanques) + NumPeons(negres))) << 1;
	for (; a != 0; a &= a - 1, i++) {
		c = lsb(a);
		fila = ROW(c);
		columna = COL(c);
		if (jo == blanques)
			punt += valorposiciotorres[c];
		else
			punt += valorposiciotorres[63 - c];

		//torre guanya valor com menys peons hi ha
		punt += guatorres;

		atacs = estat_actual->atacstorre[jo][i];

		Bitboard atacsxray = atacs;
		if (atacs & (Tdames(jo) | Ttorres(jo))) {
			atacsxray = AtacsTorre(c, TTotespeces ^ (Tdames(jo) | Ttorres(jo)));
			ie->atacs_color_amb_xray[jo] |= atacsxray;
		}

		//mobilitat
		//atacs de la peça. treure atacs peons. 
		atacsrei2 = atacs & (~ie->atacspeons_per_mobilitat[jo]); //** ho feia diferent. no cal 
		//número de caselles atacades restants
		j = popcount(atacsrei2);
		//si peça està clavada, menys mobilitat
		if (j > 0 && (ie->claven[opo] & Bit(c)))
			j--;
		if (j < 0)
			j = 0;
		punt += mobilitatt[j];
		ie->vmobilitat[jo] += mobilitatt[j];

		if (fila == fila7) {
			if (ROW(ie->rei[opo]) == fila8) {
				punt += BonusTorreTallaRei7a; //només afecta al final
			}
			if (filaant == fila7)
				punt += FerPun(15, 15); //torres doblades a 7a
		}
		filaant = fila;

		bool oberta = false;
		bool semioberta = false;
		Bitboard peopropidavant = (CasellesDavant[jo][c] & Tpeons(jo));
		bool atac = false;
		if (!peopropidavant) {
			punt2 = 0;
			Bitboard zz = Tpeons(opo) & Columnes[columna];
			if (!zz) {
				punt2 += BonusTorreColumnaOberta;
				oberta = true;
				if (colant != -1 && columna == colant)
					punt2 += BonusTorresDoblades;
			}
			else {
				semioberta = true;
				if (estat_actual->atacspeons[opo] & zz)
					punt2 += FerPun(4, -1);
				else {
					punt2 += BonusTorreColumnaSemiOberta;
				}
			}
			j = abs(columna - COL(ie->rei[opo]));
			if (j < 2 || (j == 2 && oberta))
				ie->atacs_columnes[opo]++;

			if (Tdames(opo) & Columnes[columna])
				punt += BonusTorreMiraDama;
			punt += punt2;
		}
		else {
			if (abs(columna - COL(ie->rei[opo])) < 2 && fila_relativa(jo, ROW(lsb(peopropidavant))) > 3 && (estat_actual->atacspeons[opo] & peopropidavant))
				atac = true; //està obrint columna sobre el rei
		}

		colant = columna;

		//seguretat del rei contrari
		atacsrei2 = atacs & ie->bitboard_atacsrei[opo];  //no cal precalcultorre
		if (atacsrei2) {
			j = popcount(atacsrei2);
			ie->pes_total_atacs[opo] += PesAtacTorre * j;
			j = popcount(atacsrei2 & (~ie->atacs_color_amb_xray[opo]));  //treure les caselles defensades
			ie->num_total_atacs[opo] += j;
			atac = true;
		}
		else if (PrecalculTorre[ie->rei[opo]] & Bit(c)) {
			ie->num_total_atacs[opo] += !varis_bits((Tpeons(blanques) | Tpeons(negres)) & Entre[ie->rei[opo]][c]);
			atac += (oberta || semioberta);
		}
		atacsrei2 = atacsxray & ie->bitboard_casellesrei[opo];
		if (atacsrei2) {
			ie->atacs_costat[opo] += popcount(atacsrei2)*PesAtacTorre;
			atac = true;
		}
		ie->num_peces_ataquen[opo] += atac;
		ie->ataquen_torres[opo] += atac;
	}
	return punt;
}


//positiu vol dir penalització
template<e_colors jo>
int avaluacio_rei(tss * RESTRICT ss, struct InfoEval *ie) {
	Estat_Analisis *estat_actual = ss->estat_actual;
	int i = 0, r, c, pos, x, y, z;
	const e_colors opo = (jo == blanques ? negres : blanques);

	int AtacsQuadre = 0;
	int ReiAvancat = 0;

	Bitboard atacsrei, b;
	pos = ie->rei[jo];
	r = ROW(pos);
	c = COL(pos);
	tHashPeons* thp = ie->thp;
	if (Tdames(opo)) {
		if (c >= 2 && c <= 5) { //no: if (c >= 3 && c <= 5) {
			if (jo == blanques) {
				if (!(estat_actual->enroc & PotEnrocarBlancCurt)) {
					i += PenalitzacioReiCentreSenseEnroc;
					ie->DiferenciaDesenvolupament--;
				}
				if (c != 2 && !(estat_actual->enroc & PotEnrocarBlancLlarg)) {
					i += PenalitzacioReiCentreSenseEnroc;
					ie->DiferenciaDesenvolupament--;
				}
			}
			else {
				if (!(estat_actual->enroc & PotEnrocarNegreCurt)) {
					i += PenalitzacioReiCentreSenseEnroc;
					ie->DiferenciaDesenvolupament++;
				}
				if (c != 2 && !(estat_actual->enroc & PotEnrocarNegreLlarg)) {
					i += PenalitzacioReiCentreSenseEnroc;
					ie->DiferenciaDesenvolupament++;
				}
			}
		}
		i += thp->SeguretatRei[jo];
	}

	x = r;
	int PeonsDavant;
	int valorpecescontrarisensepeons;
	if (jo == negres) {
		x = 7 - r;
		PeonsDavant = thp->Passats >> 56;
		valorpecescontrarisensepeons = ie->vpb_np;
	}
	else {
		PeonsDavant = thp->Passats & 0xFF;
		valorpecescontrarisensepeons = ie->vpn_np;
	}
	if (x > 1 || (x == 1 && PeonsDavant >= 3))
		ReiAvancat = 1;

	//si el rei no pot moure, penalització
	if (!(ie->bitboard_casellesrei[jo] & ~Tpeces(jo) & (~estat_actual->atacs[opo])))
		i += 5;

	Bitboard quadrerei = 0;
	if (Tdames(opo) || valorpecescontrarisensepeons >= 1400) {
		//quadre de 4x4 on es troba el rei
		if (c < 3)
			quadrerei = 0xF0F0F0F0F0F0F0FULL; //columnes a-d
		else {
			if (c > 4)
				quadrerei = 0xF0F0F0F0F0F0F0F0ULL; //columnes e-h
			else
				quadrerei = 0x3C3C3C3C3C3C3C3CULL; //columnes c-f
		}
		Bitboard bbb = jo == blanques ? 0xFFFFFFFFULL : 0xFFFFFFFF00000000ULL;
		quadrerei = quadrerei & bbb;
		AtacsQuadre = popcount(estat_actual->atacs[opo] & quadrerei);
	}
	else {
		AtacsQuadre = 0;
		ReiAvancat = 0;
	}

	if ((ie->num_peces_ataquen[jo] > 0 || AtacsQuadre > 3 || ReiAvancat || PeonsDavant > 1))
	{

		i += BaseSeguretatRei + AtacsQuadre * MulAtacsQuadre + ReiAvancat * MulReiAvancat;
		//si està trencant amb un peó prop del rei, incrementar valor atac
		int atacapeons = 0;
		if (quadrerei) {
			if (estat_actual->atacspeons[opo] & Tpeons(jo) & quadrerei) {
				i += 7; //5
				atacapeons = 1;
			}
			//si avançant pot trencar. Ho vaig desfer i a 35 segons era més bo, peró a 66 semblava igual o un pel pitjor
			if (ShiftPerPeons(jo, estat_actual->atacspeons[opo], 8) & Tpeons(jo) & quadrerei) {
				i += 3; //3 //no: 5
				atacapeons = 1;
			}
			//caselles dèbils prop rei (3a fila), i no àlfil
			if (!ReiAvancat) {
				Bitboard qq = (quadrerei & (FILA3 | FILA6)) & (~estat_actual->atacspeons[jo]);
				if (varis_bits(qq) && (estat_actual->atacs[opo] & qq)) {
					bool alfildefensa = ((estat_actual->atacsalfils[jo] & qq) == qq);
					//i += 5;
					if (!alfildefensa) {
						i += 15; //bo: 15 //no: 17, 20, 25
					}
				}
			}
		}
		//escacs contacte torres
		b = ie->atacs_color_amb_xray[jo] | estat_actual->atacsdames[jo] | estat_actual->atacspeons[jo]; //ie.AtacsPecesMenysRei[blanques] =
		//caselles no defensades al costat del rei blanc, que no estiguin ocupades per peces del contrari
		b = ie->bitboard_casellesrei[jo] & ~b & ~Tpeces(opo);

		atacsrei = b & estat_actual->atacstorres[opo];
		atacsrei &= PrecalculTorre[pos];

		x = 0;
		if (atacsrei) {
			atacsrei &= estat_actual->atacspeons[opo] | estat_actual->atacscaballs[opo] | estat_actual->atacsalfils[opo] | estat_actual->atacsdames[opo] | ie->bitboard_casellesrei[opo]; //es treuen les torres perquè estan suportant la seva pròpia casella d'escac
			if (atacsrei) {
				x = EscacContacteTorre * popcount(atacsrei) * (estat_actual->mou == opo ? 2 : 1); //si pot fer escac i li toca, més penalització
			}
		}

		//escacs contacte dama
		atacsrei = b & estat_actual->atacsdames[opo];
		if (atacsrei) {
			atacsrei &= (ie->atacs_color_amb_xray[opo] | estat_actual->atacspeons[opo]);
			if (atacsrei) {
				x += EscacContacteDama * popcount(atacsrei) * (estat_actual->mou == opo ? 2 : 1); //si pot fer escac i li toca, més penalització
			}
		}

		int EscacsContacte = x;
		if (EscacsContacte >= MaxEscacsContacte)
			EscacsContacte = MaxEscacsContacte - 1;
		i += EscacsContacte * MulEscacsContacte;

		x = 0;
		//escacs a distància
		Bitboard segures = (~(Tpeces(opo) | estat_actual->atacs[jo]));
		Bitboard segures2 = (segures & (PrecalculTorre[pos] | PrecalculAlfil[pos])) & (estat_actual->atacsdames[opo] | estat_actual->atacstorres[opo] | estat_actual->atacsalfils[opo]);

		if (segures2) {
			Bitboard b1 = AtacsTorre(pos, TTotespeces) & segures2;
			Bitboard b2 = AtacsAlfil(pos, TTotespeces) & segures2;

			if (b1 || b2) {
				//de dama
				b = (b1 | b2) & estat_actual->atacsdames[opo];
				x += (b) ? EscacDama * popcount(b) : 0;
			}

			if (b1) {
				//de torre
				b = b1 & estat_actual->atacstorres[opo];

				int w = (b) ? EscacTorre * popcount(b) * popcount(b) : 0;
				x += w;
				//si no havia posat que les torres atacaven, peró tenen un escac, sí que ataquen
				if (!ie->ataquen_torres[jo] && w)
					ie->num_peces_ataquen[jo]++;
			}

			if (b2) {
				//d'alfil
				b = b2 & estat_actual->atacsalfils[opo];
				x += (b) ? EscacAlfil * popcount(b) : 0;
			}
		}

		//de caball
		b = AtacsCaball[pos] & estat_actual->atacscaballs[opo] & segures;
		x += (b) ? EscacCaball * popcount(b) : 0;

		int EscacsDistancia = x;
		if (EscacsDistancia >= MaxEscacsDistancia)
			EscacsDistancia = MaxEscacsDistancia - 1;
		i += EscacsDistancia * MulEscacsDistancia;

		int AtacsCostat = ie->atacs_costat[jo];
		if (AtacsCostat >= MaxAtacsCostat)
			AtacsCostat = MaxAtacsCostat - 1;
		i += AtacsCostat * MulAtacsCostat;

		int NumAtacants = ie->num_peces_ataquen[jo];
		if (valorpecescontrarisensepeons >= 1400) {
			atacsrei = estat_actual->atacspeons[opo] & ie->bitboard_casellesrei[jo];
			if (atacsrei) {
				int jj = popcount(atacsrei);
				NumAtacants += jj;
				if (jj > 2)
					NumAtacants += jj;

				jj = popcount(atacsrei & (~ie->atacs_color_amb_xray[jo]));
				if (jj) {
					ie->num_total_atacs[jo] += jj;  //treure les caselles defensades
					ie->pes_total_atacs[jo]++;
				}
			}
			if (popcount(Tpeons(opo) & quadrerei) > 1)
				NumAtacants++;
		}

		if (!ie->ataquen_dames[jo] && popcount(estat_actual->atacsdames[opo] & quadrerei) > 2)
			NumAtacants++;

		int PesAtacs = (ie->num_total_atacs[jo] * ie->pes_total_atacs[jo]) >> 1;
		if (PesAtacs >= MaxPesAtacs)
			PesAtacs = MaxPesAtacs - 1;
		i += PesAtacs * MulPesAtacs;

		if (NumAtacants >= MaxNumAtacants)
			NumAtacants = MaxNumAtacants - 1;
		i += NumAtacants * MulNumAtacants;

		//si hi ha moltes peces defensant, menys penalització
		int NumDefensant = popcount(ie->atacs_color_amb_xray[jo] & ie->bitboard_casellesrei[jo]);
		if (NumDefensant >= MaxNumDefensant)
			NumDefensant = MaxNumDefensant - 1;
		i += NumDefensant * MulNumDefensant;

		if (NumAtacants > 2 || (NumAtacants == 2 && PesAtacs > 8 && EscacsDistancia > 0 && AtacsCostat > 8)) {
			int jj = (AtacsCostat * AtacsQuadre);
			i += (jj >> 1) + (jj >> 2);
		}

		if (ie->atacs_columnes[jo])
			i += 1 << (ie->atacs_columnes[jo] << 1);

		//penalització rei 2a fila i no té peo al costat llarg
		if ((thp->flags & Bit(jo))
			&& Tdames(opo) && Ttorres(opo))
			i += 10;

		//diagonal gran dèbil
		if (c > 4 && fila_relativa(jo, r) < 3) {
			if (jo == blanques) {
				if (!(Talfils(jo) & (~QuadresNegres)) && (Talfils(opo) & (~QuadresNegres))
					&& !((Tpeons(blanques) | Tpeons(negres)) & (C6 | D5 | E4 | F3 | G2)))
					i += 10;
			}
			else {
				if (!(Talfils(jo) & QuadresNegres) && (Talfils(opo) & QuadresNegres)
					&& !((Tpeons(blanques) | Tpeons(negres)) & (C3 | D4 | E5 | F6 | G7)))
					i += 10;
			}
		}
		if (c < 3 && fila_relativa(jo, r) < 3) {
			if (jo == blanques) {
				if (!(Talfils(jo) & QuadresNegres) && (Talfils(opo) & QuadresNegres)
					&& !((Tpeons(blanques) | Tpeons(negres)) & (F6 | E5 | D4 | C3 | B2)))
					i += 10;
			}
			else {
				if (!(Talfils(jo) & (~QuadresNegres)) && (Talfils(opo) & (~QuadresNegres))
					&& !((Tpeons(blanques) | Tpeons(negres)) & (F3 | E4 | D5 | C6 | B7)))
					i += 10;
			}
		}

		//contrari té més peons davant del rei. idea hakkapelitta
		if (Tdames(opo) && !(c == 3 || c == 4)) {
			x = min(3, popcount(Tpeons(opo) & AvancPeonsPassats[jo][pos])) - (3 - PeonsDavant);
			if (x > 0)
				i += mulpeonsdavantcontrari * x;
		}

		if (thp->flags & (jo == blanques ? HashPeonsFlagPenalitzacioSemiBloqueigCentralBlanques : HashPeonsFlagPenalitzacioSemiBloqueigCentralNegres))
			i += penalitzaciosemibloqueigcentral;

		//reduïr potència atac si: pocs atacants, (cap menor atacant i alguna pròpia defensant i no escacs contacte) o pot menjar dama
		if (i - BaseSeguretatRei > minpenalitzacioreiperdisminuirsiunatacant && (
			NumAtacants + atacapeons <= 1
			|| (EscacsContacte == 0 && !((estat_actual->atacsalfils[opo] | estat_actual->atacscaballs[opo]) & quadrerei) && ((estat_actual->atacsalfils[jo] | estat_actual->atacscaballs[jo]) & quadrerei))
			|| estat_actual->mou == jo && Tdames(opo) && !estat_actual->escac && (estat_actual->atacs[jo] & Tdames(opo))
			)) {
			i = i - ((i - BaseSeguretatRei) >> 2);
		}

		if (NumAtacants > 2 && (thp->flags & HashPeonsFlagCentreBloquejat))
			i += penalitzaciosibloqueigcentral;

		if (PunI(i) > 0) {
			if (jo == blanques)
				x = PunI(vp_rei[ie->rei[blanques]]);
			else
				x = PunI(vp_rei[63 - ie->rei[negres]]);
			if (x > 0)
				i += x; //si el rei està malament, treure-li el bonus
			//reduïr potència atac si dama no ataca
			if (!(estat_actual->atacsdames[opo] & (c < 4 ? COLUMNAA + COLUMNAB + COLUMNAC + COLUMNAD : COLUMNAE + COLUMNAF + COLUMNAG + COLUMNAH)))
				i -= 15;
		}
	}
	else {
		if (AtacsQuadre == 1)
			AtacsQuadre = 0;
		i += BaseSeguretatRei + AtacsQuadre*MulAtacsQuadre;
	}
	//si contrari mínim dama + torre, sumar algo del valor al final
	if (valorpecescontrarisensepeons >= 1750)
		i = FerPun(i, i >> 3);
	return i;
}

template<e_colors jo>
Bitboard CalcularPeonsAmbSuport(tss * RESTRICT ss) {
	//per mirar si el peo té suport
	Bitboard a = (Tpeons(jo) & (~COLUMNAA)) >> 1;
	Bitboard PeonsAmbSuport = Tpeons(jo) & a;
	a = (Tpeons(jo) & (~COLUMNAH)) << 1;
	PeonsAmbSuport |= Tpeons(jo) & a;

	if (jo == blanques) {
		a = (Tpeons(jo) & (~COLUMNAH)) << 9;
		PeonsAmbSuport |= Tpeons(jo) & a;
		a = (Tpeons(jo) & (~COLUMNAA)) << 7;
	}
	else {
		a = (Tpeons(jo) & (~COLUMNAA)) >> 9;
		PeonsAmbSuport |= Tpeons(jo) & a;
		a = (Tpeons(jo) & (~COLUMNAH)) >> 7;
	}
	PeonsAmbSuport |= Tpeons(jo) & a;
	return PeonsAmbSuport;
}

int FilaPassat[8];
int CandidatAPassat[8] = { 0, 0, 0, 131074, 327686, 524300, 786451, 1048604 };
int Connectat[2][8] = { { 0, 2, 4, 6, 22, 29, 53, 0 }, { 0, 3, 6, 15, 25, 39, 75, 0 } }; //enfila, fila

template<e_colors jo>
int avaluacio_peons_hash(tss * RESTRICT ss, struct InfoEval *ie, Bitboard *bloquejats) {
	int i, j, w, r, avant;
	const e_colors opo = (jo == blanques ? negres : blanques);
	uint8_t i8, davantpeo, darrerapeo;
	Bitboard a, veins, suportat, bons, passatsdebils;
	bool aillat, peonsdavantoponent, doblat;
	i = 0;

	tHashPeons* thp = ie->thp;
	thp->pawnAttacksSpan[jo] = 0;
	bons = passatsdebils = 0;
	Bitboard PeonsAmbSuport = CalcularPeonsAmbSuport<jo>(ss);
	for (a = Tpeons(jo); a != 0; a &= a - 1) {
		avant = i;
		i8 = lsb(a);
		thp->pawnAttacksSpan[jo] |= AtacsdePeons[jo][i8];
		w = COL(i8);
		r = ROW(i8);
		if (jo == blanques) {
			j = vp_peons[63 - i8];
			davantpeo = i8 + 8;
			darrerapeo = i8 - 8;
		}
		else {
			j = vp_peons[i8];
			davantpeo = i8 - 8;
			darrerapeo = i8 + 8;
		}
		i += j;
		veins = Tpeons(jo) & FilesAdjacentsBB[w];
		aillat = !veins;
		peonsdavantoponent = Tpeons(opo) & CasellesDavant[jo][i8];
		suportat = veins & Files[ROW(darrerapeo)];
		if (aillat)
			i -= Aillat[peonsdavantoponent][w];

		if (!suportat && !aillat)
			i -= PenalitzacioPeoNoSuportat;
		//peons doblats
		Bitboard PeonsMeusColumna = Tpeons(jo) & Columnes[w];
		if (aillat && msb(PeonsMeusColumna) != lsb(PeonsMeusColumna)) {  //el de davant o el de darrera
			i -= PenalitzacioPeoDoblatAillat;
		}

		bool enfila = veins & Files[r];
		bool connectat = false;
		if (suportat || enfila) {
			connectat = true;
			int z = Connectat[!!enfila][fila_relativa(jo, r)];
			if (peonsdavantoponent)
				z = z >> 1;
			if (varis_bits(suportat))
				z += z >> 1;
			i += FerPun(z, z >> 1);
		}

		if ((!(Tpeons(opo) & AvancPeonsPassats[jo][i8])) && !(Tpeons(jo) & CasellesDavant[jo][i8])/*doblat*/) { //si és passat //** potser marcar-ho al fer moviment, per anar mès ràpid
			//si hi ha un peo doblat a la columna, que sigui el de davant
			thp->Passats |= Bit(i8);
			if ((jo == blanques ? r > 3 : r < 4) && (PeonsAmbSuport & Bit(i8))) {
				i += BonificacioPassatAmbSuport;
			}
			i += FilaPassat[w];
			if (!aillat && !connectat && !(Tpeons(jo) & (AtacsdePeons[opo][davantpeo])))
				passatsdebils |= Bit(i8); //passat dèbil
		}
		else {
			//candidats a passat
			if (!peonsdavantoponent) {
				if (VarisBits(AtacsdePeons[jo][i8] & Tpeons(opo)))
					goto nocandidatpassat;
				if (Tpeons(jo) & (veins & Files[r]))
					goto sicandidatpassat; //és l'altre peó el passer candidate, al sacrificar aquest
				if (jo == blanques && r > 2) { //a 2a fila no el considero candidat
					//1r mirar columna esquerra
					if (w > 0
						&& (Tpeons(blanques) & (Bit(i8 - 9) | Bit(i8 - 17)))
						&& !(Tpeons(negres) & (Bit(i8 - 1) | Bit(i8 - 9)))
						&& !(ss->estat_actual->atacspeons[negres] & (Bit(i8 - 1) | Bit(i8 - 9)))
						)
						goto sicandidatpassat;
					//columna dreta
					if (w < 7
						&& (Tpeons(blanques) & (Bit(i8 - 7) | Bit(i8 - 15)))
						&& !(Tpeons(negres) & (Bit(i8 + 1) | Bit(i8 - 7)))
						&& !(ss->estat_actual->atacspeons[negres] & (Bit(i8 + 1) | Bit(i8 - 7)))
						)
						goto sicandidatpassat;
				}
				if (jo == negres && r < 5) { //a 2a fila no el considero candidat
					//1r mirar columna esquerra
					if (w > 0
						&& (Tpeons(negres) & (Bit(i8 + 7) | Bit(i8 + 15)))
						&& !(Tpeons(blanques) & (Bit(i8 - 1) | Bit(i8 + 7)))
						&& !(ss->estat_actual->atacspeons[blanques] & (Bit(i8 - 1) | Bit(i8 + 7)))
						)
						goto sicandidatpassat;
					//columna dreta
					if (w < 7
						&& (Tpeons(negres) & (Bit(i8 + 9) | Bit(i8 + 17)))
						&& !(Tpeons(blanques) & (Bit(i8 + 1) | Bit(i8 + 9)))
						&& !(ss->estat_actual->atacspeons[blanques] & (Bit(i8 + 1) | Bit(i8 + 9)))
						)
						goto sicandidatpassat;
				}
				goto nocandidatpassat;
			sicandidatpassat:
				i += CandidatAPassat[fila_relativa(jo, r)];
				if (jo == blanques)
					thp->flags |= HashPeonsFlagBlanquesCandidataPassat;
				else
					thp->flags |= HashPeonsFlagNegresCandidataPassat;
			}
		nocandidatpassat:
			if ((Tpeons(opo) & Bit(davantpeo))) {
				//disminuir puntuació base peons bloquejats
				if (PunI(j) > 0)
					i -= FerPun(PunI(j) >> 2, 0);
				if (jo == blanques)
					*bloquejats |= Bit(davantpeo) | Bit(i8); //es marca per blanques i negres de cop
			}
			//si peo dèbil
			if (!aillat && !connectat && !(Tpeons(jo) & (AtacsdePeons[opo][davantpeo]))) { //si no té als costats o (als costats darrera) cap peó del propi color, dèbil
				thp->Debils |= Bit(i8);
				if (Tpeons(opo) & Bit(davantpeo)) { //si la casella de devant està ocupada per un peó, té menys penalització
					i -= PenalitzacioPeoOcupaDavantDebil;
				}
				else {
					i -= PenalitzacioPeoDebil[w];
					//si a més està en una columna semi oberta, més penalització
					if (!peonsdavantoponent)
						;// i -= PenalitzacioExtraPeoDebilColumnaSemiOberta;
					else {
						//si no pot avaçar immediatament per tenir dos peons rivals atacant al davant, tb considerar-lo bloquejat
						if (w > 0 && w < 7 && (jo == blanques ? r < 5 : r>2) && ss->tau.c[i8 + Avancar(jo) + Avancar(jo) - 1] == afegir_color_a_peca(PeoBlanc, opo) && ss->tau.c[i8 + Avancar(jo) + Avancar(jo) + 1] == afegir_color_a_peca(PeoBlanc, opo))
							*bloquejats |= Bit(i8);
					}
				}
				//si l'estan atacant, més penalització
				//dèbil al costat de columna oberta i atac peo contrari al davant, més penalització
				if (!varis_bits(veins) && (ss->estat_actual->atacspeons[opo] & Bit(davantpeo))) {
					if (
						(w > 0 && !((Tpeons(blanques) | Tpeons(negres)) & Columnes[w - 1]))
						||
						(w < 7 && !((Tpeons(blanques) | Tpeons(negres)) & Columnes[w + 1]))
						)
						i -= penalizacio_peo_debil_costat_columna_oberta;
				}

			}
		}

		if (Tpeons(opo) & AtacsPeo[jo][i8]) {
			i += Lever[peonsdavantoponent][fila_relativa(jo, r)];
		}

		int h = PunI(i - avant);
		if (h < -10 && peonsdavantoponent)
			h += 5;

		//puntuació màxima peo = 45
		if (h > 45)
			h -= h - 45;

		if (h > 35) {
			bons |= Bit(i8);
		}

		i = FerPun(PunI(avant) + h, PunF(i));
	}

	//si la puntuació del peó és molt bona però ha deixat un peó dèbil al costat, menys bonus
	for (a = bons; a != 0; a &= a - 1) {
		i8 = lsb(a);
		if (AtacsPeo[opo][i8] & (thp->Debils | passatsdebils) & Tpeons(jo)) {
			i -= FerPun(7, 0);
		}
	}

	//bonus 2 peons centrals quan rival no peons col D ni E
	if (!(Tpeons(opo) & (COLUMNAD | COLUMNAE)) && ((Tpeons(jo) & 0x808000000) && (Tpeons(jo) & 0x1010000000)))
		i += FerPun(15, 15);

	return i;
}

int passat_avancat_no_control_coronacio = -3;

template<e_colors jo>
int avaluacio_peons_2(tss * RESTRICT ss, struct InfoEval *ie) {
	Estat_Analisis *estat_actual = ss->estat_actual;
	Bitboard a, b, c;
	int davantpeo, i8, i, z, z2, z3;

	const e_colors opo = (jo == blanques ? negres : blanques);
	if (jo == blanques)
		davantpeo = 8;
	else
		davantpeo = -8;
	i = 0;
	for (a = (ie->thp->Passats & mascarapassatsdebils) & Tpeons(jo); a != 0; a &= a - 1) {
		i8 = lsb(a);

		z = valorposiciopeonspassats[jo][i8];

		c = CasellesDavant[jo][i8];
		b = c & estat_actual->atacs[jo];
		//si peces suportant avanç, més bonificació
		if (b == c) {
			z += FerPun(PunI(z) >> 2, PunF(z) >> 2) + FerPun(PunI(z) >> 3, PunF(z) >> 3); //24
		}
		else {
			if (b & Bit(i8 + davantpeo)) { //bonus si controla casella següent peo
				z += FerPun(PunI(z) >> 3, PunF(z) >> 3) + FerPun(PunI(z) >> 4, PunF(z) >> 4); //12
			}
			else {
				//penalització si no controla casella avanç i el rival sí (amb atac o peça)
				if ((Tpeces(opo) | estat_actual->atacs[opo]) & Bit(i8 + davantpeo))
					z -= FerPun(0, PunF(z) >> 4); //4
			}
			//si no controla casella coronació i peo avançat, algo menys de valor
			bool fer = false;
			if (jo == blanques) {
				if ((ROW(i8) == 6 && estat_actual->mou == negres) || (ROW(i8) == 5 && !(b & Bit(i8 + 16)))) {
					fer = true;
				}
			}
			else {
				if ((ROW(i8) == 1 && estat_actual->mou == blanques) || (ROW(i8) == 2 && !(b & Bit(i8 - 16)))) {
					fer = true;
				}
			}
			if (fer)
				z -= FerPun(/*PunI(z) >> 3*/0, PunF(z) >> 3); //8;
		}

		bool potreduirvalor = false;
		//si caselles davant ocupades per peces rivals
		if ((c & Tpeces(opo))) {
			potreduirvalor = true;
			z -= FerPun(PunI(z) >> 2, PunF(z) >> 2) + FerPun(PunI(z) >> 3, PunF(z) >> 3); //24
			if (c & Trei(opo))
				z -= bonificacio_rei_parant_peo_passat_final;
			if (c & Tdames(opo))
				z += penalitzacio_dama_parant_peo_passat_final; //penalització per la dama, doncs és menys activa
		}
		else {
			Bitboard hh = CasellesDavant[opo][i8] & (Ttorres(opo) | Tdames(opo));
			if (hh & AtacsTorre(i8, TTotespeces)) {
				z -= FerPun(PunI(z) >> 1, PunF(z) >> 1); //32;
			}

			if (!(c & estat_actual->atacs[opo])) { //si no hi ha control de cap peça contraria de les caselles d'avanç, més penalització
				z += PenalitzacioExtraPeoPassatNoControlatFinal;
				int numpecesopo = popcount(Tpeces(opo) ^ Tpeons(opo));
				//si el rival té poques peces, doblar valor peo passat no controlat
				if (numpecesopo < 3) {
					z += FerPun(0, PunF(valorposiciopeonspassats[jo][i8]) >> 2);  //provar posar valor mg!!
				}
			}
			else {
				potreduirvalor = true;
			}
		}

		z -= penalitzacio_rei_contrari_proper_peons_passats_final[Distancia(ie->rei[opo], i8)];
		z += bonificacio_rei_proper_peons_passats_final[Distancia(ie->rei[jo], i8)];

		z2 = max(0, PunI(z));
		z3 = max(0, PunF(z));

		//si hi ha mínim dos caselles sense controlar, el peo no pot ser massa bo
		if (potreduirvalor && varis_bits(c ^ b)) {
			z2 = z2 - (z2 >> 2);
			z3 = z3 - (z3 >> 2);
		}

		//si atacat i no defensat, i casella de davant tb atacada o peo bloquejat, menys valor
		if (!(estat_actual->atacs[jo] & Bit(i8)) && (estat_actual->atacs[opo] & Bit(i8)) && ((estat_actual->atacs[opo] & Bit(i8 + davantpeo)) || (c & Tpeces(opo)))) {
			z2 = z2 - (z2 >> 3); //no >> 1
			z3 = z3 - (z3 >> 3); //no >> 1
		}

		//si no atacat i defensat, més valor
		if ((estat_actual->atacs[jo] & Bit(i8)) && !(estat_actual->atacs[opo] & Bit(i8))) {
			z3 += 5;
		}

		if (NumPeons(jo) < NumPeons(opo)) {
			z3 += z3 / 4;
		}

		i += FerPun(z2, z3);
	}

	for (a = ie->thp->Debils & Tpeons(jo); a != 0; a &= a - 1) {
		i8 = lsb(a);

		b = Bit(i8 + davantpeo);
		if (!(Tpeons(opo) & b)) {
			if (estat_actual->atacs[opo] & b) {
				i -= PenalitzacioPecaAtacaCasellaDavantDebil;
			}
			if (Tpeces(opo) & b) {
				i -= PenalitzacioPecaOcupaDavantDebil;
			}
		}

		//si no estan atacant la casella de davant ni està ocupada pel rival, menys penalització
		if (!(Tpeces(opo) & b) && !(estat_actual->atacs[opo] & b)) {
			i += BonificacioDebilnoAtacadaNiOcupada;
		}

		//si l'estan atacant, més penalització
		if (estat_actual->atacs[opo] & Bit(i8)) { //** deia AtacsPecesMenysReicontrari
			i -= PenalitzacioPecaAtacaDebil;
		}
	}
	return i;
}

int avaluacio_peons(tss * RESTRICT ss, struct InfoEval *ie){
	Estat_Analisis *estat_actual = ss->estat_actual;
	tHashPeons* thp = ie->thp;
	if (thp->hash != estat_actual->hashpeons) {
		thp->hash = estat_actual->hashpeons;
		thp->Passats = thp->Debils = thp->flags = 0;
		Bitboard bloquejats = 0;
		thp->score = avaluacio_peons_hash<blanques>(ss, ie, &bloquejats) - avaluacio_peons_hash<negres>(ss, ie, &bloquejats);

		int c = COL(ie->rei[blanques]);
		int r = ROW(ie->rei[blanques]);
		int cn = COL(ie->rei[negres]);
		int rn = ROW(ie->rei[negres]);

		thp->score += vp_rei[ie->rei[blanques]];
		thp->score -= vp_rei[63 - ie->rei[negres]];

		//si el rei està lluny en vertical d'on hi ha els peons al final
		Bitboard peons = Tpeons(blanques) | Tpeons(negres);
		if (peons) {
			int maxrow = ROW(msb(peons));
			int minrow = ROW(lsb(peons));
			int distanciavreiblanc = r;
			int distanciavreinegre = rn;
			if (distanciavreiblanc < minrow)
				distanciavreiblanc = minrow - distanciavreiblanc;
			else
				if (distanciavreiblanc > maxrow)
					distanciavreiblanc = distanciavreiblanc - maxrow;
				else
					distanciavreiblanc = 0;
			if (distanciavreinegre < minrow)
				distanciavreinegre = minrow - distanciavreinegre;
			else
				if (distanciavreinegre > maxrow)
					distanciavreinegre = distanciavreinegre - maxrow;
				else
					distanciavreinegre = 0;
			if (abs(distanciavreiblanc - distanciavreinegre) > 1) {
				int penalitzacio = PenalitzacioDistanciaReiColumnaPeonsAux[abs(distanciavreiblanc - distanciavreinegre)];
				if (distanciavreiblanc > distanciavreinegre)
					thp->score -= FerPun(0, penalitzacio);
				else
					thp->score += FerPun(0, penalitzacio);
			}
		}

		//peonsdavant. S'aprofita Passats per evitar afegir més memòria a hash peons
		Bitboard b2 = Tpeons(blanques) & FilesDavant[blanques][r];
		Bitboard pd = 3 - popcount(b2 & ie->bitboard_casellesrei[blanques]) + (!!(Tpeons(blanques) & AvancPeonsPassats[blanques][ie->rei[blanques]] & estat_actual->atacspeons[negres])); //atacs peons rivals compten com un peó menys
		thp->Passats |= pd;
		thp->SeguretatRei[blanques] = pd * MulPeonsDavant;

		b2 = Tpeons(negres) & FilesDavant[negres][rn];
		pd = ((3 - popcount(b2 & ie->bitboard_casellesrei[negres]) + (!!(Tpeons(negres) & AvancPeonsPassats[negres][ie->rei[negres]] & estat_actual->atacspeons[blanques])))); //atacs peons rivals compten com un peó menys
		thp->Passats |= pd << 56;
		thp->SeguretatRei[negres] = pd * MulPeonsDavant;

		//si el rei està lluny en horitzontal d'on hi ha els peons al final
		int estpb = (!!(Tpeons(blanques) & COLUMNAA)) |   //** potser marcar-ho al fer moviment, per anar mès ràpid
			(!!(Tpeons(blanques) & COLUMNAB)) << 1 |
			(!!(Tpeons(blanques) & COLUMNAC)) << 2 |
			(!!(Tpeons(blanques) & COLUMNAD)) << 3 |
			(!!(Tpeons(blanques) & COLUMNAE)) << 4 |
			(!!(Tpeons(blanques) & COLUMNAF)) << 5 |
			(!!(Tpeons(blanques) & COLUMNAG)) << 6 |
			(!!(Tpeons(blanques) & COLUMNAH)) << 7;
		thp->score -= PenalitzacioIllesPeons[estpb];
		int estpn = (!!(Tpeons(negres) & COLUMNAA)) |
			(!!(Tpeons(negres) & COLUMNAB)) << 1 |
			(!!(Tpeons(negres) & COLUMNAC)) << 2 |
			(!!(Tpeons(negres) & COLUMNAD)) << 3 |
			(!!(Tpeons(negres) & COLUMNAE)) << 4 |
			(!!(Tpeons(negres) & COLUMNAF)) << 5 |
			(!!(Tpeons(negres) & COLUMNAG)) << 6 |
			(!!(Tpeons(negres) & COLUMNAH)) << 7;
		thp->score += PenalitzacioIllesPeons[estpn];

		int estp = estpb | estpn;
		thp->score -= PenalitzacioDistanciaReiColumnaPeons[c][estp];
		thp->score += PenalitzacioDistanciaReiColumnaPeons[cn][estp];

		//hi ha d'haver algun bloquejat a les dues columnes del centre
		if (popcount(bloquejats & QuadresCentrals4x4) >= 4 && ((bloquejats & QuadresCentrals4x4) & (COLUMNAD | COLUMNAE)))
			thp->flags |= HashPeonsFlagCentreBloquejat;
		else
			thp->flags &= ~HashPeonsFlagCentreBloquejat;

		//per penalització rei 2a fila i no té peo al costat llarg
		if (r == 1
			&& ((c > 3 && !(Tpeons(blanques) & (Bit(ie->rei[blanques] - 1) | Bit(ie->rei[blanques] - 2))))
			|| (c <= 3 && !(Tpeons(blanques) & (Bit(ie->rei[blanques] + 1) | Bit(ie->rei[blanques] + 2))))))
			thp->flags |= HashPeonsFlagReiBlancPerill2a;
		else
			thp->flags &= ~HashPeonsFlagReiBlancPerill2a;

		if (rn == 6
			&& ((cn > 3 && !(Tpeons(negres) & (Bit(ie->rei[negres] - 1) | Bit(ie->rei[negres] - 2))))
			|| (cn <= 3 && !(Tpeons(negres) & (Bit(ie->rei[negres] + 1) | Bit(ie->rei[negres] + 2))))))
			thp->flags |= HashPeonsFlagReiNegrePerill2a;
		else
			thp->flags &= ~HashPeonsFlagReiNegrePerill2a;


		thp->flags &= ~(HashPeonsFlagPenalitzacioSemiBloqueigCentralBlanques | HashPeonsFlagPenalitzacioSemiBloqueigCentralNegres);
		if (c > 4) {
			//peo blanc a e3 i f2, i peo negre a e4
			if ((Tpeons(blanques) & 0x102000ULL) == 0x102000ULL && (Tpeons(negres) & E4))
				thp->flags |= HashPeonsFlagPenalitzacioSemiBloqueigCentralBlanques;
		}
		if (cn > 4) {
			//peo negre a e6 i f7, i peo blanc a e5
			if ((Tpeons(negres) & 0x20100000000000ULL) == 0x20100000000000ULL && (Tpeons(blanques) & E5))
				thp->flags |= HashPeonsFlagPenalitzacioSemiBloqueigCentralNegres;
		}

		//avaluar estructura peons pròpia davant rei
		int y = ie->rei[blanques];
		if (ROW(y) > 0) {
			y += Avancar(negres); //també costats
		}
		if ((y == 0 || y == 7) && (Tpeons(negres) & Bit(y + 8)))
			thp->SeguretatRei[blanques] -= 15; //peó contrari davant del rei el protegeig
		Bitboard b = AvancPeonsPassats[blanques][y] & Tpeons(blanques); //peons davant i al costat del rei
		if (!b)
			thp->SeguretatRei[blanques] += 40; //35 //no: 30
		else {
			int x = lsb(b);
			b ^= Bit(x);
			y = lsb(b);
			b ^= Bit(y);
			int z = lsb(b);
			b ^= Bit(z);
			if (x == 0)
				x = 255; //perquè no quadri mai
			if (y == 0)
				y = 255; //perquè no quadri mai
			if (z == 0)
				z = 255; //perquè no quadri mai
			int w = abs(x - z);
			x = abs(x - y);
			y = abs(y - z);
			if (x != 7 && x != 1 && x != 9 && y != 7 && y != 1 && y != 9 && w != 7 && w != 1 && w != 9) //cap grup de dos peons lligats
				thp->SeguretatRei[blanques] += 25;  //no: 30
		}

		//atac de peons
		b = AvancPeonsPassats[blanques][ie->rei[blanques]] & Tpeons(negres) & (FILA2 | FILA3 | FILA4 | FILA5); //peons contraris davant i al costat del rei, avançats
		if (varis_bits(b)) {
			thp->SeguretatRei[blanques] += 5;
			if (popcount(b) >= 3)
				thp->SeguretatRei[blanques] += 10;
		}


		//si rei a flanc i peça contraria a f3/c3, penalització extra
		if (c < 4 && (Tpeons(negres) & C3)) {
			thp->SeguretatRei[blanques] += 10;
			if (!(Tpeons(blanques) & COLUMNAA) || !(Tpeons(blanques) & COLUMNAB))
				thp->SeguretatRei[blanques] += 5;
		}
		if (c >= 4 && (Tpeons(negres) & F3)) {
			thp->SeguretatRei[blanques] += 10;
			if (!(Tpeons(blanques) & COLUMNAH) || !(Tpeons(blanques) & COLUMNAG))
				thp->SeguretatRei[blanques] += 5;
		}

		//avaluar estructura peons pròpia davant rei
		y = ie->rei[negres];
		if (ROW(y) < 7) {
			y += Avancar(blanques); //també costats
		}
		if ((y == 56 || y == 63) && (Tpeons(blanques) & Bit(y - 8)))
			thp->SeguretatRei[negres] -= 15; //peó contrari davant del rei el protegeig
		b = AvancPeonsPassats[negres][y] & Tpeons(negres); //peons davant i al costat del rei
		if (!b)
			thp->SeguretatRei[negres] += 40; //35 //no: 30
		else {
			int x = msb(b);
			b ^= Bit(x);
			y = msb(b);
			b ^= Bit(y);
			int z = msb(b);
			b ^= Bit(z);
			if (x == 0)
				x = 255; //perquè no quadri mai
			if (y == 0)
				y = 255; //perquè no quadri mai
			if (z == 0)
				z = 255; //perquè no quadri mai
			int w = abs(x - z);
			x = abs(x - y);
			y = abs(y - z);
			if (x != 7 && x != 1 && x != 9 && y != 7 && y != 1 && y != 9 && w != 7 && w != 1 && w != 9) //cap grup de dos peons lligats
				thp->SeguretatRei[negres] += 25;  //no: 30
		}

		//atac de peons
		b = AvancPeonsPassats[negres][ie->rei[negres]] & Tpeons(blanques) & (FILA7 | FILA6 | FILA5 | FILA4); //peons contraris davant i al costat del rei, avançats
		if (varis_bits(b)) {
			thp->SeguretatRei[negres] += 5;
			if (popcount(b) >= 3)
				thp->SeguretatRei[negres] += 10;
		}


		//si rei a flanc i peça contraria a f6/c6, penalització extra
		if (cn < 4 && (Tpeons(blanques) & C6)) {
			thp->SeguretatRei[negres] += 10;
			if (!(Tpeons(negres) & COLUMNAA) || !(Tpeons(negres) & COLUMNAB))
				thp->SeguretatRei[negres] += 5;
		}
		if (cn >= 4 && (Tpeons(blanques) & F6)) {
			thp->SeguretatRei[negres] += 10;
			if (!(Tpeons(negres) & COLUMNAH) || !(Tpeons(negres) & COLUMNAG))
				thp->SeguretatRei[negres] += 5;
		}

	}
	int i = thp->score + avaluacio_peons_2<blanques>(ss, ie) - avaluacio_peons_2<negres>(ss, ie);

	//Peons que potser no poden ser parats, en finals de només peons
	if (ie->vpb_np == 0 && ie->vpn_np == 0)
	{
		Bitboard a;
		if ((a = ((thp->Passats & mascarapassatsdebils) & Tpeons(blanques))) != 0)
			i += int(ROW(msb(a))) * FerPun(0, bonificacio_base_peons_passats_potser_no_parables);

		if ((a = ((thp->Passats & mascarapassatsdebils) & Tpeons(negres))) != 0)
			i -= int(7 - ROW(lsb(a))) * FerPun(0, bonificacio_base_peons_passats_potser_no_parables);
	}

	if (Tdames(blanques) && Tdames(negres))
		i = FerPun(PunI(i) - (PunI(i) >> 3), PunF(i));
	return i;
}


static const int atacspeoapeca[] = {
	0, 0, 0, 0, FerPun(70, 50), FerPun(70, 50), FerPun(50, 45), FerPun(50, 45), 0, 0, FerPun(90, 90), FerPun(90, 90),
	FerPun(80, 80), FerPun(80, 80), 0, 0
};
static const int atacsmenorpeca[] = {
	0, 0, FerPun(0, 12), FerPun(0, 12), FerPun(17, 16), FerPun(17, 16), FerPun(18, 19), FerPun(18, 19), 0, 0, FerPun(28, 42), FerPun(28, 42),
	FerPun(19, 45), FerPun(19, 45), 0, 0
};
static const int atacstorrepeca[] = {
	0, 0, FerPun(0, 8), FerPun(0, 8), FerPun(15, 23), FerPun(15, 23), FerPun(15, 21), FerPun(15, 21), 0, 0, FerPun(0, 12), FerPun(0, 12),
	FerPun(14, 18), FerPun(14, 18), 0, 0
};

static const int atacsderei[] = {
	FerPun(0, 24), FerPun(0, 52)
};

int avaluacioamenaces(tss * RESTRICT ss, struct InfoEval *ie) {
	Bitboard b, weak, b2, b3, b4;
	int i = 0;
	Estat_Analisis *estat_actual = ss->estat_actual;

	//atacs a peces no defensades
	i -= popcount(estat_actual->atacs[negres] & (Tpeces(blanques) & ~estat_actual->atacs[blanques])) * FerPun(19, 10);
	i += popcount(estat_actual->atacs[blanques] & (Tpeces(negres) & ~estat_actual->atacs[negres])) * FerPun(19, 10);

	int c = ss->estat_actual->mou;

	//peces negres sense peons
	b2 = Tpeces(negres) ^ (Tpeons(negres) | Trei(negres));
	//atacs de peons a peces excepte a peons
	b = estat_actual->atacspeons[blanques] & b2;
	if (b) {
		if (c == blanques)
			b = Tpeons(blanques);
		else
			//peons defensats o no atacats
			b = Tpeons(blanques) & (estat_actual->atacs[blanques] | (~estat_actual->atacs[negres]));
		if (b) {
			if (c == negres) //treure els peons que el contrari amenaça amb els seus peons, si juga el contrari
				b &= ~estat_actual->atacspeons[negres];
			if (b) {
				//atacs d'aquests peons a peces
				b3 = (((b & (~COLUMNAH)) << 9) | ((b & (~COLUMNAA)) << 7)) & b2;
				for (; b3 != 0; b3 &= b3 - 1)
					i += atacspeoapeca[ss->tau.c[lsb(b3)]];
			}
		}
	}

	//possibles atacs de peons si avancen
	b = (
		(((Tpeons(blanques) & ~FILA7) | ((((Tpeons(blanques) & FILA2)) << 8) & ~TTotespeces)) << 8) //peons que no estàn a 7a, i afegir peons de segona no bloquejats a 3a que poden avançar dues caselles
		& ~TTotespeces) & ~estat_actual->atacspeons[negres];
	//que casella d'avanç estigui defensada per nosaltres, o que no estigui atacada pel rival
	b &= (estat_actual->atacs[blanques] | ~estat_actual->atacs[negres]);
	if (b) {
		//atacs d'aquests peons a peces
		b = (((b & (~COLUMNAH)) << 9) | ((b & (~COLUMNAA)) << 7)) & b2 & ~estat_actual->atacspeons[blanques]; //que no estiguin ja atacades pels nostres peons
		if (b) {
			i += popcount(b) * FerPun(15, 9);
		}
	}

	//atacs de menors
	b2 |= Tpeons(negres) & ~estat_actual->atacspeons[negres]; //afegir peons no defensats
	b3 = b2 & (estat_actual->atacscaballs[blanques] | estat_actual->atacsalfils[blanques]);
	for (; b3 != 0; b3 &= b3 - 1)
		i += atacsmenorpeca[ss->tau.c[lsb(b3)]];

	//atacs de torres
	b3 = b2 & (estat_actual->atacstorres[blanques]);
	for (; b3 != 0; b3 &= b3 - 1)
		i += atacstorrepeca[ss->tau.c[lsb(b3)]];

	//atacs de rei
	b2 = Tpeces(negres) & ~estat_actual->atacspeons[negres] & AtacsRei[ie->rei[blanques]];
	if (b2)
		i += atacsderei[!!VarisBits(b2)];


	//peces blanques sense peons
	b2 = Tpeces(blanques) ^ (Tpeons(blanques) | Trei(blanques));
	//atacs de peons a peces excepte a peons
	b = estat_actual->atacspeons[negres] & b2;
	if (b) {
		if (c == negres)
			b = Tpeons(negres);
		else
			//peons defensats o no atacats
			b = Tpeons(negres) & (estat_actual->atacs[negres] | (~estat_actual->atacs[blanques]));
		if (b) {
			if (c == blanques) //treure els peons que el contrari amenaça amb els seus peons, si juga el contrari
				b &= ~estat_actual->atacspeons[blanques];
			//atacs d'aquests peons a peces
			if (b) {
				b3 = (((b & (~COLUMNAH)) >> 7) | ((b & (~COLUMNAA)) >> 9)) & b2;
				for (; b3 != 0; b3 &= b3 - 1)
					i -= atacspeoapeca[ss->tau.c[lsb(b3)]];
			}
		}
	}

	//possibles atacs de peons si avancen
	b = (
		(((Tpeons(negres) & ~FILA2) | ((((Tpeons(negres) & FILA7)) >> 8) & ~TTotespeces)) >> 8) //peons que no estàn a 2a, i afegir peons de 7a no bloquejats a 6a que poden avançar dues caselles
		& ~TTotespeces) & ~estat_actual->atacspeons[blanques];
	//que casella d'avanç estigui defensada per nosaltres, o que no estigui atacada pel rival
	b &= (estat_actual->atacs[negres] | ~estat_actual->atacs[blanques]);
	if (b) {
		//atacs d'aquests peons a peces
		b = (((b & (~COLUMNAH)) >> 7) | ((b & (~COLUMNAA)) >> 9)) & b2 & ~estat_actual->atacspeons[negres]; //que no estiguin ja atacades pels nostres peons
		if (b) {
			i -= popcount(b) * FerPun(15, 9);
		}
	}

	//atacs de menors
	b2 |= Tpeons(blanques) & ~estat_actual->atacspeons[blanques]; //afegir peons no defensats
	b3 = b2 & (estat_actual->atacscaballs[negres] | estat_actual->atacsalfils[negres]);
	for (; b3 != 0; b3 &= b3 - 1)
		i -= atacsmenorpeca[ss->tau.c[lsb(b3)]];

	//atacs de torres
	b3 = b2 & (estat_actual->atacstorres[negres]);
	for (; b3 != 0; b3 &= b3 - 1)
		i -= atacstorrepeca[ss->tau.c[lsb(b3)]];

	//atacs de rei
	b2 = Tpeces(blanques) & ~estat_actual->atacspeons[blanques] & AtacsRei[ie->rei[negres]];
	if (b2)
		i -= atacsderei[!!VarisBits(b2)];
	return i;
}

int imbalanc_material(tss * RESTRICT ss, struct InfoEval *ie){
	Estat_Analisis *estat_actual = ss->estat_actual;
	int x, y, z = 0, z2 = 0;

	//compensar una mica diferència material si hi ha molta diferència de mobilitat
	if ((x = PunI(estat_actual->valorpeces[blanques] - estat_actual->valorpeces[negres])) != 0) {
		y = PunI(ie->vmobilitat[blanques] - ie->vmobilitat[negres]);
		if (!(x < 0 && y < 0) || (x > 0 && y > 0)) { //no compensar si diferència material i mobilitat son del mateix signe
			if (abs(y) > 50)  {//prova 20, 35, 60 i era practicament igual
				z = CompensacioEntreMaterialIMobilitat1;
				z2 = CompensacioEntreMaterialIMobilitat1Final;
			}
			if (abs(y) > 100) {
				z = CompensacioEntreMaterialIMobilitat2;
				z2 = CompensacioEntreMaterialIMobilitat2Final;
			}
			if (abs(y) > 150) {
				z = CompensacioEntreMaterialIMobilitat3;
				z2 = CompensacioEntreMaterialIMobilitat3Final;
			}
			if (x > 0 && y < 0) {
				z = -z;
				z2 = -z2;
			}
		}

	}

	if (abs(x) >= vcaballsee)  //no cal per diferències tan grans
		return 0;
	if (ie->taulamat == NULL || ie->taulamat->imbalanc == NULL)
		return 0;
	y = abs((int)(estat_actual->numpeces[PeoBlanc] - estat_actual->numpeces[PeoNegre]));
	if (y > 5)
		y = 5;
	y = ie->taulamat->imbalanc[y] * ie->taulamat->signe;

	//si molt material, menys compensació
	int z3 = 0;
	if (ie->taulamat->imbalanc != caball_i_peons_per_torre) {
		if (ie->vpb > 2200)
			z3 = 20;
	}
	if (x > 0) //avantatja material blanques
		y += FerPun(z3, z3);
	else
		y -= FerPun(z3, z3);

	return y + FerPun(z, z2);
}

int ti(int j) {
	return PunI(j);
}

int tf(int j) {
	return PunF(j);
}

int torg(Moviment m) {
	return Origen(m);
}

int tdes(Moviment m) {
	return Desti(m);
}

int espai(tss * RESTRICT ss, struct InfoEval *ie) {
	int b = 0, n = 0;
	Bitboard segures;
	Estat_Analisis *estat_actual = ss->estat_actual;

	Bitboard peons = Tpeons(blanques);
	if (peons) {
		segures = 0x183C3C7E00 & (~estat_actual->atacs[negres]) & (estat_actual->atacs[blanques] | Tpeces(blanques));
		peons |= (peons >> 8) | (peons >> 16) | (peons >> 24);
		peons &= segures;
		b = popcount(segures) + popcount(peons);
	}

	peons = Tpeons(negres);
	if (peons) {
		segures = 0x7E3C3C18000000 & (~estat_actual->atacs[blanques]) & (estat_actual->atacs[negres] | Tpeces(negres));
		peons |= (peons << 8) | (peons << 16) | (peons << 24);
		peons &= segures;
		n = popcount(segures) + popcount(peons);
	}
	int i = (b - n) >> 1;
	return i;
}

int avaluacioreiblanques;
int avaluacio(tss * RESTRICT ss) {
	Estat_Analisis *estat_actual = ss->estat_actual;
	struct InfoEval ie;
	Bitboard b;

	ie.vmobilitat[blanques] = ie.vmobilitat[negres] = 0;

	ie.escalar_avaluacio_final = 0;
	ie.vpb = PunI(estat_actual->valorpeces[blanques]);
	ie.vpb_np = ie.vpb - vpeo * estat_actual->numpeces[PeoBlanc];
	ie.vpn = PunI(estat_actual->valorpeces[negres]);
	ie.vpn_np = ie.vpn - vpeo * estat_actual->numpeces[PeoNegre];
	ie.thp = &HashPeons[estat_actual->hashpeons % NumHashPeons];

	if (estat_actual->numpeces[DamaBlanca] >= maxdamab || estat_actual->numpeces[TorreBlanca] >= maxtorreb || estat_actual->numpeces[AlfilBlanc] >= maxalfilb || estat_actual->numpeces[CaballBlanc] >= maxcaballb ||
		estat_actual->numpeces[DamaNegre] >= maxdaman || estat_actual->numpeces[TorreNegre] >= maxtorren || estat_actual->numpeces[AlfilNegre] >= maxalfiln || estat_actual->numpeces[CaballNegre] >= maxcaballn) {
		ie.taulamat = NULL;
	}
	else
		ie.taulamat = &taulamaterial[estat_actual->numpeces[DamaBlanca]][estat_actual->numpeces[TorreBlanca]][estat_actual->numpeces[AlfilBlanc]][estat_actual->numpeces[CaballBlanc]]
		[estat_actual->numpeces[DamaNegre]][estat_actual->numpeces[TorreNegre]][estat_actual->numpeces[AlfilNegre]][estat_actual->numpeces[CaballNegre]];

	//** _mm_prefetch((char *)&HashPeons[estat_actual->hashpeons & PeonsHashMask], _MM_HINT_T0);
	if (estat_actual->clavatsrival == Tota1BB)
		estat_actual->clavatsrival = marcar_clavats(ss, 1, oponent(estat_actual->mou));
	if (estat_actual->mou == blanques) {
		ie.claven[blanques] = estat_actual->clavatsrival;
		ie.claven[negres] = estat_actual->clavats;
	}
	else {
		ie.claven[blanques] = estat_actual->clavats;
		ie.claven[negres] = estat_actual->clavatsrival;
	}
	ie.DiferenciaDesenvolupament = 0;
	ie.rei[blanques] = lsb(Trei(blanques));
	ie.rei[negres] = lsb(Trei(negres));
	int punt = 0;

	//Mirar si algun final estàndard (fins a dama i 2 peons)
	if (ie.vpb == 0 || ie.vpn == 0 ||
		(ie.vpb <= 1300 && ie.vpn <= 1300)) {
		bool sortir;
		punt = mirar_si_final_especialitzat(ss, &ie, &sortir);
		if (sortir)
			goto acabar;
		punt = FerPun(punt, punt);
	}

	//per mobilitat
	ie.atacspeons_per_mobilitat[blanques] = (estat_actual->atacspeons[negres] & ~((Tpeces(blanques) | Tpeces(negres)) - Tpeons(negres))) | Tpeons(blanques) | Trei(blanques);
	ie.atacspeons_per_mobilitat[negres] = (estat_actual->atacspeons[blanques] & ~((Tpeces(blanques) | Tpeces(negres)) - Tpeons(blanques))) | Tpeons(negres) | Trei(negres);

	//seguretat rei
	ie.pes_total_atacs[blanques] = ie.num_total_atacs[blanques] = ie.pes_total_atacs[negres] = ie.num_total_atacs[negres] = ie.atacs_costat[blanques] = ie.atacs_costat[negres] = 0;
	ie.num_peces_ataquen[blanques] = ie.num_peces_ataquen[negres] = ie.ataquen_torres[blanques] = ie.ataquen_torres[negres] = ie.atacs_columnes[blanques] = ie.atacs_columnes[negres] = 0;
	ie.ataquen_dames[blanques] = ie.ataquen_dames[negres] = 0;
	ie.atacs_color_amb_xray[blanques] = estat_actual->atacscaballs[blanques] | estat_actual->atacsalfils[blanques] | estat_actual->atacstorres[blanques];
	ie.atacs_color_amb_xray[negres] = estat_actual->atacscaballs[negres] | estat_actual->atacsalfils[negres] | estat_actual->atacstorres[negres];
	ie.bitboard_casellesrei[blanques] = AtacsRei[ie.rei[blanques]];
	ie.bitboard_atacsrei[blanques] = ie.bitboard_casellesrei[blanques] & ~(estat_actual->atacspeons[blanques] | Tpeons(negres));
	ie.bitboard_casellesrei[negres] = AtacsRei[ie.rei[negres]];
	ie.bitboard_atacsrei[negres] = ie.bitboard_casellesrei[negres] & ~(estat_actual->atacspeons[negres] | Tpeons(blanques));

	punt += estat_actual->valorpeces[blanques] - estat_actual->valorpeces[negres];

	int z;
	//bonus per canvi de peces si avantatja material  //** revisar si això té sentit
	if (PunI(punt) >= vpeosee) {
		punt += FerPun(ValorSimplificacio2, ValorSimplificacio2);
	}
	if (-PunI(punt) >= vpeosee) {
		punt -= FerPun(ValorSimplificacio2, ValorSimplificacio2);
	}

	if (estat_actual->mou == blanques)
		punt += BonusPerQuiMou;
	else
		punt -= BonusPerQuiMou;

	punt += avaluacio_peons(ss, &ie);
	if (Tcaballs(blanques)) {
		punt += avaluacio_caballs<blanques>(ss, &ie);
	}
	else
		ie.ValorCaballs[blanques] = 0;
	if (Tcaballs(negres)) {
		punt -= avaluacio_caballs<negres>(ss, &ie);
	}
	else
		ie.ValorCaballs[negres] = 0;
	if (Talfils(blanques)) {
		punt += avaluacio_alfils<blanques>(ss, &ie);
		//falta de desenvolupament
		if (Talfils(blanques) & C1)
			ie.DiferenciaDesenvolupament--;
		if (Talfils(blanques) & D1)
			ie.DiferenciaDesenvolupament--;
		if (Talfils(blanques) & E1)
			ie.DiferenciaDesenvolupament--;
		if (Talfils(blanques) & F1)
			ie.DiferenciaDesenvolupament--;
	}
	if (Talfils(negres)) {
		punt -= avaluacio_alfils<negres>(ss, &ie);
		if (Talfils(negres) & C8)
			ie.DiferenciaDesenvolupament++;
		if (Talfils(negres) & D8)
			ie.DiferenciaDesenvolupament++;
		if (Talfils(negres) & E8)
			ie.DiferenciaDesenvolupament++;
		if (Talfils(negres) & F8)
			ie.DiferenciaDesenvolupament++;
	}
	if (Ttorres(blanques))
		punt += avaluacio_torres<blanques>(ss, &ie);
	if (Ttorres(negres))
		punt -= avaluacio_torres<negres>(ss, &ie);
	if (Tdames(blanques))
		punt += avaluacio_dames<blanques>(ss, &ie);
	if (Tdames(negres))
		punt -= avaluacio_dames<negres>(ss, &ie);

	//menor darrera peo
	b = (((Talfils(blanques) | Tcaballs(blanques)) & FILA1a4) << 8) & (Tpeons(blanques) | Tpeons(negres));
	punt += b ? MenorDarreraPeo * popcount(b) : 0;
	b = (((Talfils(negres) | Tcaballs(negres)) & FILA5a8) >> 8) & (Tpeons(blanques) | Tpeons(negres));
	punt -= b ? MenorDarreraPeo * popcount(b) : 0;

	int vrei; vrei = avaluacio_rei<blanques>(ss, &ie);
	punt -= vrei;
	//compensar algo material si rei malament
	int difpeces; difpeces = PunI(estat_actual->valorpeces[blanques] - estat_actual->valorpeces[negres]);
	if (difpeces > 50) {
		if (PunI(vrei) > 300)
			punt -= difpeces >> 2;
		if (PunI(vrei) > 200)
			punt -= difpeces >> 3;
		if (PunI(vrei) > 100)
			punt -= difpeces >> 4;
	}

	vrei = avaluacio_rei<negres>(ss, &ie);
	punt += vrei;
	//compensar algo material si rei malament
	difpeces = difpeces * -1;
	if (difpeces > 50) {
		if (PunI(vrei) > 300)
			punt += difpeces >> 2;
		if (PunI(vrei) > 200)
			punt += difpeces >> 3;
		if (PunI(vrei) > 100)
			punt += difpeces >> 4;
	}

	punt += avaluacioamenaces(ss, &ie);
	punt += imbalanc_material(ss, &ie);

	if (estat_actual->numpeces[AlfilBlanc] + estat_actual->numpeces[TorreBlanca] != estat_actual->numpeces[AlfilNegre] + estat_actual->numpeces[TorreNegre]) {
		z = popcount((Tpeons(blanques) | Tpeons(negres)) & (Columnes[3] | Columnes[4])); //núm de peons centrals
		if (estat_actual->numpeces[AlfilBlanc] + estat_actual->numpeces[TorreBlanca] > estat_actual->numpeces[AlfilNegre] + estat_actual->numpeces[TorreNegre]) {
			punt += BonusMenysPeonsCentralsSiMesTorresoAlfils[z];
		}
		else {
			punt -= BonusMenysPeonsCentralsSiMesTorresoAlfils[z];
		}
	}
	int xx;
	xx = espai(ss, &ie);
	punt += xx;
	if (ie.thp->flags & HashPeonsFlagCentreBloquejat) {
		//si la posició està molt tancada, qui té més espai té avantatja
		if (xx > 0) {
			punt += 3;
		}
		if (xx < 0) {
			punt -= 3;
		}
	}

	bool peonsdosflancs;
	peonsdosflancs = ((Tpeons(blanques) | Tpeons(negres)) & Mascara_abc) && ((Tpeons(blanques) | Tpeons(negres)) & Mascara_fgh);
	if (estat_actual->numpeces[AlfilBlanc] != estat_actual->numpeces[AlfilNegre]) {  //** quan és alfil per torre (qualitat de menys) o alfil i caball per torre, potser no té molt sentit això. es pot senzillament comprovar que el núm de torres siguin iguals
		//alfil és millor si hi ha peons als dos flancs
		if (peonsdosflancs && !(ie.thp->flags & HashPeonsFlagCentreBloquejat)) {
			punt += FerPun(PunI(BonusAlfilPerPeonsEnDosFlancs)*estat_actual->numpeces[AlfilBlanc], PunF(BonusAlfilPerPeonsEnDosFlancs)*estat_actual->numpeces[AlfilBlanc]);
			punt -= FerPun(PunI(BonusAlfilPerPeonsEnDosFlancs)*estat_actual->numpeces[AlfilNegre], PunF(BonusAlfilPerPeonsEnDosFlancs)*estat_actual->numpeces[AlfilNegre]);
		}
		if (estat_actual->numpeces[AlfilBlanc] > 1) {
			if (PunI(ie.ValorCaballs[negres]) > MinValorCaballsPerBonusvsAlfilsBons || (ie.thp->flags & HashPeonsFlagCentreBloquejat)) {
				punt += BonusParellaAlfilsvsCaballsBons;
			}
			else
				punt += BonusParellaAlfils;
			if (PunF(ie.ValorCaballs[negres]) > MinValorCaballsPerBonusvsAlfilsBons || (ie.thp->flags & HashPeonsFlagCentreBloquejat)) {
				punt += BonusParellaAlfilsvsCaballsBonsFinal;
			}
			else
				punt += BonusParellaAlfilsFinal;

			if (estat_actual->numpeces[PeoNegre] > 5)
				punt -= PenalitzacioParellaAlfilsSiMoltsPeons;
			if (estat_actual->numpeces[AlfilNegre] == 0) {
				if (estat_actual->numpeces[CaballNegre] == 0)
					punt += BonusParellaAlfilsNoCanviables;
				if (estat_actual->numpeces[CaballNegre] == estat_actual->numpeces[AlfilBlanc])
					punt += FerPun(3, 3); //2 alfils vs 2 caballs
			}
		}
		if (estat_actual->numpeces[AlfilNegre] > 1) {
			if (PunI(ie.ValorCaballs[blanques]) > MinValorCaballsPerBonusvsAlfilsBons || (ie.thp->flags & HashPeonsFlagCentreBloquejat)) {
				punt -= BonusParellaAlfilsvsCaballsBons;
			}
			else
				punt -= BonusParellaAlfils;
			if (PunF(ie.ValorCaballs[blanques]) > MinValorCaballsPerBonusvsAlfilsBons || (ie.thp->flags & HashPeonsFlagCentreBloquejat)) {
				punt -= BonusParellaAlfilsvsCaballsBonsFinal;
			}
			else
				punt -= BonusParellaAlfilsFinal;

			if (estat_actual->numpeces[PeoBlanc] > 5)
				punt += PenalitzacioParellaAlfilsSiMoltsPeons;
			if (estat_actual->numpeces[AlfilBlanc] == 0) {
				if (estat_actual->numpeces[CaballBlanc] == 0)
					punt -= BonusParellaAlfilsNoCanviables;
				if (estat_actual->numpeces[CaballBlanc] == estat_actual->numpeces[AlfilNegre])
					punt -= FerPun(3, 3); //2 alfils vs 2 caballs
			}
		}
	}

	if (((Tcaballs(blanques) & B1) || (Talfils(blanques) & C1)) && (Ttorres(blanques) & A1)) {
		punt -= PenalitzacioTorreICaballposicioOriginal;
		ie.DiferenciaDesenvolupament--;
	}
	if (((Tcaballs(blanques) & G1) || (Talfils(blanques) & F1)) && (Ttorres(blanques) & H1)) {
		punt -= PenalitzacioTorreICaballposicioOriginal;
		ie.DiferenciaDesenvolupament--;
	}
	if (((Tcaballs(negres) & B8) || (Talfils(negres) & C8)) && (Ttorres(negres) & A8)) {
		punt += PenalitzacioTorreICaballposicioOriginal;
		ie.DiferenciaDesenvolupament++;
	}
	if (((Tcaballs(negres) & G8) || (Talfils(negres) & F8)) && (Ttorres(negres) & H8)) {
		punt += PenalitzacioTorreICaballposicioOriginal;
		ie.DiferenciaDesenvolupament++;
	}

	ie.DiferenciaDesenvolupament = ie.DiferenciaDesenvolupament > 10 ? 10 : ie.DiferenciaDesenvolupament;
	ie.DiferenciaDesenvolupament = ie.DiferenciaDesenvolupament < -10 ? -10 : ie.DiferenciaDesenvolupament;

	if (ie.DiferenciaDesenvolupament > 0)
		punt += PenalitzacioDiferenciaDesenvolupament[ie.DiferenciaDesenvolupament];
	else if (ie.DiferenciaDesenvolupament < 0)
		punt -= PenalitzacioDiferenciaDesenvolupament[-ie.DiferenciaDesenvolupament];


	if ((Tpeons(blanques) & E2) || (Tpeons(blanques) & D2))
		punt -= FerPun(4, 0);

	if ((Tpeons(negres) & E7) || (Tpeons(negres) & D7))
		punt += FerPun(4, 0);

	//si no té material per guanyar, no pot tenir avantatja
	if (estat_actual->numpeces[PeoBlanc] == 0 && ie.vpb < AmbMenysMaterialNoEsPotGuanyar && PunF(punt) > 0) {
		punt = 0;
		if (estat_actual->numpeces[PeoNegre] > 0)
			punt = -1;  //si el rival té peons, anar a menjar-los per aconseguir el 0.
	}
	if (estat_actual->numpeces[PeoNegre] == 0 && ie.vpn < AmbMenysMaterialNoEsPotGuanyar && PunF(punt) < 0) {
		punt = 0;
		if (estat_actual->numpeces[PeoBlanc] > 0)
			punt = 1;  //si el rival té peons, anar a menjar-los per aconseguir el 0.
	}

	//alfils diferent color amb més peces
	if (!ie.escalar_avaluacio_final && ie.vpb <= 2000 && ie.vpn <= 2000
		&& estat_actual->numpeces[AlfilBlanc] == 1
		&& estat_actual->numpeces[AlfilNegre] == 1
		&& !(ie.vpb_np == valfil && ie.vpn_np == valfil)) {
		//alfils de diferent color
		bool difcolor;
		Bitboard tots2 = Talfils(blanques) | Talfils(negres);
		difcolor = (tots2 & QuadresNegres) && (tots2 & ~QuadresNegres);
		if (difcolor)
			punt = FerPun(PunI(punt), (int)(PunF(punt) * 50 / 64));
	}

	//Ponderar de cara al final
	if (PunI(punt) != PunF(punt)) {
		int pf; pf = PunF(punt);
		int jjj = estat_actual->numpeces[PeoBlanc] + estat_actual->numpeces[PeoNegre];
		if (jjj < 6) {
			if (jjj < 3) {
				pf = pf - (pf >> 2) - (pf >> 3);
			}
			else
				pf = pf - (pf >> 2);
		}
		int fase;
		if (ie.taulamat == NULL)
			fase = 0; //per defecte suposa que moltes peces, doncs supera mínim de peces per poder agafar un index existent a taulamat
		else
			fase = ie.taulamat->phase;
		punt = ((PunI(punt) * (256 - fase)) + (pf * fase)) / 256;
	}
	else
		punt = PunI(punt);

	if (peonsdosflancs) {
		if (punt > 0)
			punt += 8; //5
		if (punt < 0)
			punt -= 8; //5
	}

	//Mirar si algun final estàndard
	if (ie.vpb <= 1500 && ie.vpn <= 1500) {
		//KRBKR
		if (ie.vpb == valfil + vtorre && ie.vpn_np == vtorre && punt > 100)
			punt = punt >> 3;  //sino creu que té molta avantatja, quan molts cops son taules

		if (ie.vpn == valfil + vtorre && ie.vpb_np == vtorre && punt < -100)
			punt = punt >> 3;

		//KRNKR
		if (ie.vpb == vcaball + vtorre && ie.vpn_np == vtorre && punt > 100)
			punt = punt >> 3;  //sino creu que té molta avantatja, quan molts cops son taules

		if (ie.vpn == vcaball + vtorre && ie.vpb_np == vtorre && punt < -100)
			punt = punt >> 3;

		//KQBKQ, KQNKQ
		if (ie.vpb == valfil + vdama && ie.vpn == vdama && punt > 100)
			punt = punt >> 3;  //sino creu que té molta avantatja, quan molts cops son taules

		if (ie.vpb == vcaball + vdama && ie.vpn == vdama && punt > 100)
			punt = punt >> 3;  //sino creu que té molta avantatja, quan molts cops son taules

		if (ie.vpn == valfil + vdama && ie.vpb == vdama && punt < -100)
			punt = punt >> 3;  //sino creu que té molta avantatja, quan molts cops son taules

		if (ie.vpn == vcaball + vdama && ie.vpb == vdama && punt < -100)
			punt = punt >> 3;  //sino creu que té molta avantatja, quan molts cops son taules

		//KRPKBP(P)
		if (ie.vpb == vtorre + vpeo && (ie.vpn == valfil + vpeo || ie.vpn == valfil + vpeo + vpeo) && punt > 70 && (ie.thp->Passats & mascarapassatsdebils) == 0)
			punt -= punt >> 2;  //sino creu que té molta avantatja, quan molts cops son taules

		if (ie.vpn == vtorre + vpeo && (ie.vpb == valfil + vpeo || ie.vpb == valfil + vpeo + vpeo) && punt < -70 && (ie.thp->Passats & mascarapassatsdebils) == 0)
			punt -= punt >> 2;  //sino creu que té molta avantatja, quan molts cops son taules

		//KBNKR, KBBKR, KNNKR
		if ((ie.vpn == vtorre && punt > 100) || (ie.vpb == vtorre && punt < -100)) {
			if ((ie.vpb == valfil + valfil || ie.vpb == vcaball + valfil ||
				ie.vpb == vcaball + vcaball)
				||
				(ie.vpn == valfil + valfil || ie.vpn == vcaball + valfil ||
				ie.vpn == vcaball + vcaball))
				punt = punt >> 2;  //sino creu que té molta avantatja, quan molts cops son taules
		}

		if (ie.vpb_np == valfil && ie.vpn_np == valfil /*&& (punt>100 || punt<-100)*/) {
			//alfils de diferent color
			bool difcolor;
			Bitboard tots2 = Talfils(blanques) | Talfils(negres);
			difcolor = (tots2 & QuadresNegres) && (tots2 & ~QuadresNegres);
			if (difcolor) {
				if (ie.thp->Passats) {
					if (VarisBits(ie.thp->Passats & Tpeons(blanques)) || (ie.thp->flags & HashPeonsFlagBlanquesCandidataPassat))
						punt = (punt >> 2) + (punt >> 1);  //sino creu que té molta avantatja, quan molts cops son taules*/
					else {
						if (VarisBits(ie.thp->Passats & Tpeons(negres)) || (ie.thp->flags & HashPeonsFlagNegresCandidataPassat))
							punt = (punt >> 2) + (punt >> 1);  //sino creu que té molta avantatja, quan molts cops son taules*/
						else
							punt = /*(punt >> 2) + */(punt >> 1);  //sino creu que té molta avantatja, quan molts cops son taules*/
					}
				}
				else
					punt = /*(punt >> 2) + */(punt >> 1);  //sino creu que té molta avantatja, quan molts cops son taules*/

			}
			//KBPKB si només hi ha un peo (o 2 i estan doblats), mirar si quasi taules
			z = -1;
			if (estat_actual->numpeces[PeoBlanc] + estat_actual->numpeces[PeoNegre] == 1) {
				if (estat_actual->numpeces[PeoBlanc] >= 1)
					z = lsb(Tpeons(blanques));
				else
					z = lsb(Tpeons(negres));
			}
			if (z != -1) {
				if (estat_actual->numpeces[PeoBlanc] >= 1) {
					if (ROW(z) < 5 && difcolor)
						punt = punt >> 3;
					else {
						if (ie.rei[negres] > z && COL(ie.rei[negres]) == COL(z)) //si rei parant peo
							punt = punt >> 3;
						else
							if (estat_actual->atacsalfils[negres] & CasellesDavant[blanques][z] && difcolor)	//alfil parant peo
								punt = punt >> 3;
					}
				}
				else {
					if (ROW(z) > 2 && difcolor)
						punt = punt >> 3;
					else {
						if (ie.rei[blanques] < z && COL(ie.rei[blanques]) == COL(z)) //si rei parant peo
							punt = punt >> 3;
						else
							if (estat_actual->atacsalfils[blanques] & CasellesDavant[negres][z] && difcolor)	//alfil parant peo
								punt = punt >> 3;
					}
				}
			}

			z = -1;
			if (estat_actual->numpeces[PeoBlanc] + estat_actual->numpeces[PeoNegre] == 2) {
				if (estat_actual->numpeces[PeoBlanc] >= 1) {
					z = msb(Tpeons(blanques));
					if (COL(z) != COL(lsb(Tpeons(blanques))))
						z = -1;
				}
				else {
					z = lsb(Tpeons(negres));
					if (COL(z) != COL(msb(Tpeons(negres))))
						z = -1;
				}
			}
			if (z != -1) { //entra aquí si peons doblats
				if (estat_actual->numpeces[PeoBlanc] >= 1) {
					if (ie.rei[negres] > z && COL(ie.rei[negres]) == COL(z)) //si rei parant peo
						punt = punt >> 3;
					else
						if ((estat_actual->atacsalfils[negres] & CasellesDavant[blanques][z]) & ie.bitboard_casellesrei[negres])	//alfil parant peo suportat pel rei
							punt = punt >> 3;
				}
				else {
					if (ie.rei[blanques] < z && COL(ie.rei[blanques]) == COL(z)) //si rei parant peo
						punt = punt >> 3;
					else
						if ((estat_actual->atacsalfils[blanques] & CasellesDavant[negres][z]) & ie.bitboard_casellesrei[blanques])	//alfil parant peo suportat pel rei
							punt = punt >> 3;
				}
			}
		}

		//** potser no fer-ho si ha fet alguna de les coses anteriors
		//més difícil guanyar si no hi ha peons
		if ((punt > 0 && estat_actual->numpeces[PeoBlanc] == 0) || (punt < 0 && estat_actual->numpeces[PeoNegre] == 0))
			punt = (punt >> 2) + (punt >> 1);
	}

acabar:
	if (ie.escalar_avaluacio_final)
		punt = (punt >> ie.escalar_avaluacio_final);
	//si prop de la regla de 50 moviments, disminuir avaluació
	if (estat_actual->jugades50 > 50) {
		if (estat_actual->jugades50 > 85)
			punt = (punt >> 2);
		else
			punt = (punt >> 2) + (punt >> 1);
	}
	else
		if (estat_actual->jugades50 > 20)
			punt = punt - (punt >> 3);

	if (estat_actual->mou == blanques) {
		estat_actual->avaluacio = punt;
		return punt;
	}
	else {
		estat_actual->avaluacio = -punt;
		return -punt;
	}
}
