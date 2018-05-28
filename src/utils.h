#ifndef INCLOS_UTILS_H
#define INCLOS_UTILS_H

#include "definicions.h"
#include <string>
#include "analisis.h"
#include <stdint.h>
#ifdef WINDOWS
#include <intrin.h>
#endif
#include <sys/timeb.h>
#include "debug.h"
#include "magics.h"

#ifdef WINDOWS
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


void afout(const char *fmt, ...);

extern std::string fenposini;
extern std::string fenposmigjoc;
extern std::string fenposmigjoc2;
extern std::string fenposmigjoc3;
extern std::string fenposfinal;
extern std::string fenposfinal2;
extern Bitboard Files[8];
extern Bitboard Columnes[8];

extern Bitboard PrecalculTorre[64];
extern Bitboard PrecalculAlfil[64];
extern Bitboard Entre[64][64];
extern Bitboard FilesDavant[2][8];
extern Bitboard Direccio[64][64];
extern Bitboard CasellesDavant[2][64];
extern Bitboard AtacsdePeons[2][64];
extern Bitboard AvancPeonsPassats[2][64];
extern Bitboard FilesAdjacentsBB[8];

void omplir_totalpecescolor(tss * RESTRICT ss);
bool inicialitzar_taulell_fen(tss * RESTRICT ss, char aFEN[]);
int moviments_en_algebraic(tss * RESTRICT ss, char * a);
int convertir_a_casella(char * a);
void omplir_bb_peces(Taulell *taux);

extern uint8_t FlagsEnroc[64];
void precalcul_flags_enroc();
void moviment_a_texte_simple(Moviment m, char *s);

extern Bitboard AtacsRei[64];
extern Bitboard AtacsCaball[64];
extern Bitboard AtacsPeo[2][64];
void precalcul_bitboards();

void perft(tss * RESTRICT ss, int n);
void divide(tss * RESTRICT ss, int n);


#ifdef ES32BITS

inline uint8_t lsb(Bitboard b) {
	unsigned long a;
	if (_BitScanForward(&a, static_cast<unsigned long>(b)))
		return a;
	if (_BitScanForward(&a, static_cast<unsigned long>(b >> 32))) {
		a += 32;
		return a;
	}
	return 0;
}

inline uint8_t msb(Bitboard b) {
	unsigned long a;
	if (_BitScanReverse(&a, static_cast<unsigned long>(b >> 32))) {
		a += 32;
		return a;
	}
	if (_BitScanReverse(&a, static_cast<unsigned long>(b)))
		return a;
	return 0;
}

#else

inline uint8_t lsb(Bitboard b) {
#ifdef WINDOWS
	register unsigned long a;
	_BitScanForward64(&a, b);
	return a;
#endif
#ifdef LINUX
	return __builtin_ctzll(b);
#endif
}

inline uint8_t msb(Bitboard b) {
#ifdef WINDOWS
	register unsigned long a;
	_BitScanReverse64(&a, b);
	return a;
#endif
#ifdef LINUX
	return 63 - __builtin_clzll(b);
#endif
}

#endif

#ifdef POPCOUNT
inline uint8_t popcount(Bitboard b) {
#ifdef WINDOWS
	return (uint8_t)__popcnt64(b);
#endif
#ifdef LINUX
	return (uint8_t)__builtin_popcountll(b);
#endif
}

#else

#ifndef ES32BITS  //windows 64 bits sense popcnt
__forceinline uint8_t popcount(Bitboard b) {
	b -= (b >> 1) & 0x5555555555555555ULL;
	b = ((b >> 2) & 0x3333333333333333ULL) + (b & 0x3333333333333333ULL);
	b = ((b >> 4) + b) & 0x0F0F0F0F0F0F0F0FULL;
	return (b * 0x0101010101010101ULL) >> 56;
}

#else  //windows 32 bits sense popcnt. Molt més ràpida aquesta funció
__forceinline uint8_t popcount(Bitboard b) {
	unsigned w = unsigned(b >> 32), v = unsigned(b);
	v -= (v >> 1) & 0x55555555;
	w -= (w >> 1) & 0x55555555;
	v = ((v >> 2) & 0x33333333) + (v & 0x33333333);
	w = ((w >> 2) & 0x33333333) + (w & 0x33333333);
	v = ((v >> 4) + v + (w >> 4) + w) & 0x0F0F0F0F;
	return (v * 0x01010101) >> 24;
}
#endif

#endif

inline unsigned t_magic_index(int c, Bitboard occ) {
	return unsigned(((occ & MagicTMask[c]) * MagicT[c]) >> MagicTShift[c]);
}

inline unsigned a_magic_index(int c, Bitboard occ) {
	return unsigned(((occ & MagicAMask[c]) * MagicA[c]) >> MagicAShift[c]);
}

#define AtacsTorre(c,occ) MagicTAttacks[c][ t_magic_index(c,occ) ]
#define AtacsAlfil(c,occ) MagicAAttacks[c][ a_magic_index(c,occ) ]
#define AtacsDama(c,occ) (AtacsTorre(c,occ) | AtacsAlfil(c,occ))

void calcular_atacs(tss * RESTRICT ss);
Bitboard marcar_clavats(tss * RESTRICT ss, bool trobar_pins, int color);
void quickSortJugades(Estat_Analisis *estat_actual);
void OrdenacioEstableJugades(Estat_Analisis *estat_actual);
int color_casella(int sq);
void assignar_avaluacions_actuals();

Bitboard atacants(tss * RESTRICT ss, uint8_t casella, Bitboard occ);
int min_atacant(tss * RESTRICT ss, e_colors jo, int to, Bitboard Atacantsmouen,
	Bitboard& ocupat, Bitboard& atacants);
void setHistory(tss * RESTRICT ss, Moviment m, bool finsjugada, int puntuacio);
void mostra_taulell(tss * RESTRICT ss);
extern char representacio_peces[ReiNegre + 1];
Bitboard atacs_desde(uint8_t p, int casella, Bitboard occ);

#define maxjugadeslmr 64
extern int8_t reduccio_lmr[64][maxjugadeslmr];
#define MaxMovimentsFutils 20
extern int MovimentsFutils[MaxMovimentsFutils];
void inicialitzar_lmr();

inline int fila_relativa(int color, int fila) {
	return fila ^ (color * 7);
}

inline Bitboard quadresdecolor(int bitquadre) {
	return (QuadresNegres & bitquadre) ? QuadresNegres : ~QuadresNegres;
}

const int8_t tauladireccions[] = {
	1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 4, 2, 4, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 4, 1, 2, 1, 4, 0, 0, 0, 0, 0,
	0, 2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 4, 1, 2, 1, 4, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 4, 2, 4, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1, 0, 0,
	0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1, 0,
	0, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 1
};

inline int8_t direccio_entre_caselles(int from, int to) {
	int offs = to + (to | 7) - from - (from | 7) + 0x77;
	return tauladireccions[offs];
}

#endif