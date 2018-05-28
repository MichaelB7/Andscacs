#ifndef INCLOS_AVALUACIO_H
#define INCLOS_AVALUACIO_H

#include "analisis.h"
#include "debug.h"
#include "hash.h"
#include "definicions.h"

extern int valorpeca[ReiNegre + 1];
extern int valorpecasee[ReiNegre + 1]; //per SEE
extern int valorpecaseeambRei[ReiNegre + 1]; //per SEE
extern int vp_rei[64];
void posar_avaluacions(struct tposicio *pz, struct tposicio2 *pz2);

extern void carregar_fitxer_avaluacions();

extern int penalitzacio_peces_penjades();
void precalcul_estructura_peons();

static inline bool varis_bits(Bitboard b)
{
	return b & (b - 1);
}

struct InfoEval {
	Bitboard claven[2];
	Bitboard atacs_color_amb_xray[2], bitboard_casellesrei[2], bitboard_atacsrei[2];
	Bitboard atacspeons_per_mobilitat[2];

	int pes_total_atacs[2], num_total_atacs[2], atacs_costat[2], num_peces_ataquen[2], ataquen_torres[2], ataquen_dames[2], atacs_columnes[2];
	int DiferenciaDesenvolupament;
	int rei[2];
	int escalar_avaluacio_final;
	tHashPeons* thp;
	struct TMaterial* taulamat;
	int vmobilitat[2];
	int ValorCaballs[2];
	int ValorAlfils[2];
	int vpb, vpn, vpb_np, vpn_np; //valor I peces blanques, negres, idem sense peons
};

template<e_colors jo>
int avaluacio_caballs(tss * RESTRICT ss, struct InfoEval *ie);
template<e_colors jo>
int avaluacio_alfils(tss * RESTRICT ss, struct InfoEval *ie);
template<e_colors jo>
int avaluacio_dames(tss * RESTRICT ss, struct InfoEval *ie);
template<e_colors jo>
int avaluacio_torres(tss * RESTRICT ss, struct InfoEval *ie);
template<e_colors jo>
int avaluacio_rei(tss * RESTRICT ss, struct InfoEval *ie);
template<e_colors jo>
int avaluacio_peons_hash(tss * RESTRICT ss, struct InfoEval *ie, Bitboard *bloquejats);
template<e_colors jo>
int avaluacio_peons_2(tss * RESTRICT ss, struct InfoEval *ie);
int avaluacio_peons(tss * RESTRICT ss, struct InfoEval *ie);
int imbalanc_material(tss * RESTRICT ss, struct InfoEval *ie);
int avaluacioamenaces(tss * RESTRICT ss, struct InfoEval *ie);
extern int avaluacioreiblanques;
extern int avaluacio(tss * RESTRICT ss);

#define AmbMenysMaterialNoEsPotGuanyar 400

#define ValorAtacsReiPeo 1  //per acumular valor de peces que ataquen el rei
#define ValorAtacsReiCaball 3  //per acumular valor de peces que ataquen el rei
#define ValorAtacsReiAlfil 3  //per acumular valor de peces que ataquen el rei
#define ValorAtacsReiTorre 5  //per acumular valor de peces que ataquen el rei
#define ValorAtacsReiDama 10  //per acumular valor de peces que ataquen el rei
#define ValorAtacsNecessari 9  //per no disminuir la penalització per rei atacat

extern int valorposiciopeces[ReiNegre + 1][64];
extern int valorposiciopecesSensenegatiunegres[ReiNegre + 1][64];
extern int valorposiciopecesfinal[ReiNegre + 1][64];
extern int BonusPerQuiMou;
extern int penalitzacio_rei_contrari_proper_peons_passats_final[8];
extern int bonificacio_rei_proper_peons_passats_final[8];
extern int BonificacioPassatAmbSuport;

extern int valorposiciopeonspassats[2][64];
extern int bonificacio_base_peons_passats_potser_no_parables;
extern int PenalitzacioTorreBloquejada;
extern int BonusTorreColumnaSemiOberta;
extern int BonusTorreColumnaOberta;
extern int BonusTorresDoblades;
extern int BonusTorreTallaRei7a;
extern int BonusTorreMiraDama;

extern int BonusParellaAlfils;
extern int BonusParellaAlfilsFinal;
extern int BonusParellaAlfilsNoCanviables;
extern int PenalitzacioParellaAlfilsSiMoltsPeons;
extern int PenalitzacioAlfilMateixColor1Peo;
extern int PenalitzacioAlfilBloquejatPeoCentral, PenalitzacioAlfilBloquejatPeoCentral2;
extern int AlfilClavantCaballvsDama;
extern int BonusAlfilPerPeonsEnDosFlancs;
extern int BonusAlfilNoPotSerExpulsatPerPeo[8];
extern int BonusAlfilSuportatPerPeo;
extern int MinValorCaballsPerBonusvsAlfilsBons;
extern int BonusParellaAlfilsvsCaballsBons;
extern int BonusParellaAlfilsvsCaballsBonsFinal;

extern int PenalitzacioPeoDebil[8]; //per columnes
extern int PenalitzacioDistanciaReiColumnaPeonsAux[8]; //distancia
extern int PenalitzacioDistanciaReiColumnaPeons[8][256]; //columna, estructura peons dels dos colors juntades amb or
extern int PenalitzacioIllesPeons[256];

extern int FilaPassat[8];

extern int PenalitzacioPeoOcupaDavantDebil;
extern int penalizacio_peo_debil_costat_columna_oberta;

extern int PenalitzacioPeoDoblatAillat;
extern int PenalitzacioPeoNoSuportat;
extern int Aillat[2][8];
extern int Lever[2][8];

extern int BonusMenysPeonsCentralsSiMesTorresoAlfils[9];

extern int MenorDarreraPeo;
extern int BonusCaballSuportatPerPeo;
extern int BonusCaballNoPotSerExpulsatPerPeo[8];
extern int PenalitzacioCaballAtrapatPeons;
extern int PenalitzacioCaballBloquejaPeoC;

extern int valorposicioalfils[64];
extern int vp_caballs[64];
extern int valorposiciotorres[64];
extern int valorposiciodama[64];

extern int seguretat_final;
extern int mulpeonsdavantcontrari;
extern int penalitzaciosemibloqueigcentral;
extern int penalitzaciosibloqueigcentral;
extern int minpenalitzacioreiperdisminuirsiunatacant;

#define MaxPesAtacs 80
#define MaxEscacsContacte 40
#define MaxEscacsDistancia 20
#define MaxAtacsCostat 50
#define MaxPeonsDavant 4
#define MaxReiAvancat 2
#define MaxNumAtacants 8
#define MaxNumDefensant 8

extern int PenalitzacioReiCentreSenseEnroc;
extern int PenalitzacioTorreICaballposicioOriginal;
extern int PenalitzacioDiferenciaDesenvolupament[11];

extern int CompensacioEntreMaterialIMobilitat1, CompensacioEntreMaterialIMobilitat1Final;
extern int CompensacioEntreMaterialIMobilitat2, CompensacioEntreMaterialIMobilitat2Final;
extern int CompensacioEntreMaterialIMobilitat3, CompensacioEntreMaterialIMobilitat3Final;

extern int alfil_per_peons[6];

extern int caball_per_peons[6];

extern int caball_i_peons_per_torre[6];

extern int alfil_i_peons_per_torre[6];

extern int alfil_i_caball_per_torre_i_peons[6];

extern int dama_per_torre_caball_i_peons[6];

extern int dama_per_torre_alfil_i_peons[6];

extern int dama_per_torre_alfil_amb_parella_i_peons[6];

extern int torre_i_peons_per_dos_alfils[6];

extern int mobilitatc[];
extern int mobilitata[];
extern int mobilitatt[];
extern int mobilitatd[];

void calcular_imbalanc();

int torg(Moviment m);
int tdes(Moviment m);
int espai(tss * RESTRICT ss, struct InfoEval *ie);

inline bool colors_diferents(Bitboard s1, Bitboard s2) {
	Bitboard tots2 = s1 | s2;
	return (tots2 & QuadresNegres) && (tots2 & ~QuadresNegres);
}

#define maxdamab 4
#define maxtorreb 4  
#define maxalfilb 4
#define maxcaballb 4

#define maxdaman 4
#define maxtorren 4
#define maxalfiln 4
#define maxcaballn 4

struct TMaterial {
	int *imbalanc;
	int phase;
	int8_t signe;
};

extern struct TMaterial taulamaterial[maxdamab][maxtorreb][maxalfilb][maxcaballb][maxdaman][maxtorren][maxalfiln][maxcaballn];

#endif