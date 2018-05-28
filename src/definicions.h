#ifndef INCLOS_DEFINICIONS_H
#define INCLOS_DEFINICIONS_H

#include <stdint.h>
#ifdef LINUX
#include <limits.h>
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
typedef uint64_t Bitboard;

#define RESTRICT __restrict

enum e_colors { blanques, negres };

enum e_peces {
	CasellaBuida, pecares1, PeoBlanc, PeoNegre, CaballBlanc, CaballNegre, AlfilBlanc, AlfilNegre, pecares8, pecares9, TorreBlanca, TorreNegre,
	DamaBlanca, DamaNegre, ReiBlanc, ReiNegre
};
enum e_numpecesgenerics { pecag0, pecag1, Peo, pecag3, Caball, pecag5, Alfil, pecag7, pecag8, pecag9, Torre, pecag11, Dama, pecag13, Rei };
enum e_numpecescurts { NumResCurt, NumPeoCurt, NumCaballCurt, NumAlfilCurt, NumTorreCurt, NumDamaCurt, NumReiCurt };
#define CasellaInvalida 127

#define peca_sense_color(peca) ((peca) & 14)
#define oponent(colo) ((colo) ^ 1)
#define afegir_color_a_peca(peca,color) ((peca) | (color))
#define Color(peca) ((peca) & 1)

#define PotEnrocarBlancCurt 1
#define PotEnrocarNegreCurt 2
#define PotEnrocarBlancLlarg 4
#define PotEnrocarNegreLlarg 8

#define MaxProfunditat 127
#define MaxPly 256 //128 jugades

typedef uint64_t ClauHash;

typedef int16_t Valor;
#define FerPun(i,f) ((i) + ((f) << 16)) //fer puntuació
#define PosarA0QuanDividirPer2 0xFFFEFFFFULL // per si fa >>1, posar a 0 el bit 16 perquè el primer bit de la puntuació de final no passi a ser el bit més significatiu de la puntuació de mig joc

#define Convertir(x,type) ((type)(x))
#define PunI(x) Convertir((x) & 0xFFFF,short)
#define PunF(x) ((((x) >> 15) & 1) + Convertir((x) >> 16,short))

#define Bit(x) ((unsigned long long)1 << (x))
#define TestBit(b, x) ((b) & Bit(x))
#define SetBit(b, x) (b) |= Bit(x)
#define COL(sq)  ((sq) & 7)
#define ROW(sq)  ((sq) >> 3)
#define FerCasella(fil,col) (((fil) << 3) + (col))
#define COLUMNAA 0x0101010101010101ULL
#define COLUMNAB 0x0202020202020202ULL
#define COLUMNAC 0x0404040404040404ULL
#define COLUMNAD 0x0808080808080808ULL
#define COLUMNAE 0x1010101010101010ULL
#define COLUMNAF 0x2020202020202020ULL
#define COLUMNAG 0x4040404040404040ULL
#define COLUMNAH 0x8080808080808080ULL
#define FILA8	 0xFF00000000000000ULL
#define FILA7	 0xFF000000000000ULL
#define FILA6	 0xFF0000000000ULL
#define FILA5	 0xFF00000000ULL
#define FILA4	 0xFF000000ULL
#define FILA3	 0xFF0000ULL
#define FILA2	 0xFF00ULL
#define FILA1	 0xFFULL
#define Mascara_abc 0x707070707070707 //columes abc a 1
#define Mascara_fgh 0xE0E0E0E0E0E0E0E0
#define Distancia(x,y) max(abs(ROW(x)-ROW(y)), abs(COL(x)-COL(y)))
#define ShiftPerPeons(me,bb,v) ((me) ? ((bb) << (v)) : ((bb) >> (v)))
#define VarisBits(b) ((b) & ((b) - 1))
#define Avancar(jo) ((jo) ? (-8) : (8))
#define Fila(me,n) ((me) ? Files[7 - (n)] : Files[n])
#define Tota1BB COLUMNAA+COLUMNAB+COLUMNAC+COLUMNAD+COLUMNAE+COLUMNAF+COLUMNAG+COLUMNAH
#define FILA1a4 0xFFFFFFFFULL
#define FILA5a8 0xFFFFFFFF00000000ULL
#define FILA3a6 0xFFFFFFFF0000ULL
#define QuadresNegres 0xAA55AA55AA55AA55ULL
#define QuadresCentrals4x4 0x3C3C3C3C0000

#define A1 ((uint64_t)1)
#define B1 ((uint64_t)1 << 1)
#define C1 ((uint64_t)1 << 2)
#define D1 ((uint64_t)1 << 3)
#define E1 ((uint64_t)1 << 4)
#define F1 ((uint64_t)1 << 5)
#define G1 ((uint64_t)1 << 6)
#define H1 ((uint64_t)1 << 7)

#define A2 ((uint64_t)1 << 8)
#define B2 ((uint64_t)1 << 9)
#define C2 ((uint64_t)1 << 10)
#define D2 ((uint64_t)1 << 11)
#define E2 ((uint64_t)1 << 12)
#define F2 ((uint64_t)1 << 13)
#define G2 ((uint64_t)1 << 14)
#define H2 ((uint64_t)1 << 15)

#define A3 ((uint64_t)1 << 16)
#define B3 ((uint64_t)1 << 17)
#define C3 ((uint64_t)1 << 18)
#define D3 ((uint64_t)1 << 19)
#define E3 ((uint64_t)1 << 20)
#define F3 ((uint64_t)1 << 21)
#define G3 ((uint64_t)1 << 22)
#define H3 ((uint64_t)1 << 23)

#define A4 ((uint64_t)1 << 24)
#define B4 ((uint64_t)1 << 25)
#define C4 ((uint64_t)1 << 26)
#define D4 ((uint64_t)1 << 27)
#define E4 ((uint64_t)1 << 28)
#define F4 ((uint64_t)1 << 29)
#define G4 ((uint64_t)1 << 30)
#define H4 ((uint64_t)1 << 31)

#define A5 ((uint64_t)1 << 32)
#define B5 ((uint64_t)1 << 33)
#define C5 ((uint64_t)1 << 34)
#define D5 ((uint64_t)1 << 35)
#define E5 ((uint64_t)1 << 36)
#define F5 ((uint64_t)1 << 37)
#define G5 ((uint64_t)1 << 38)
#define H5 ((uint64_t)1 << 39)

#define A6 ((uint64_t)1 << 40)
#define B6 ((uint64_t)1 << 41)
#define C6 ((uint64_t)1 << 42)
#define D6 ((uint64_t)1 << 43)
#define E6 ((uint64_t)1 << 44)
#define F6 ((uint64_t)1 << 45)
#define G6 ((uint64_t)1 << 46)
#define H6 ((uint64_t)1 << 47)

#define A7 ((uint64_t)1 << 48)
#define B7 ((uint64_t)1 << 49)
#define C7 ((uint64_t)1 << 50)
#define D7 ((uint64_t)1 << 51)
#define E7 ((uint64_t)1 << 52)
#define F7 ((uint64_t)1 << 53)
#define G7 ((uint64_t)1 << 54)
#define H7 ((uint64_t)1 << 55)

#define A8 ((uint64_t)1 << 56)
#define B8 ((uint64_t)1 << 57)
#define C8 ((uint64_t)1 << 58)
#define D8 ((uint64_t)1 << 59)
#define E8 ((uint64_t)1 << 60)
#define F8 ((uint64_t)1 << 61)
#define G8 ((uint64_t)1 << 62)
#define H8 ((uint64_t)1 << 63)


#define cA1 0
#define cB1 1
#define cC1 2
#define cD1 3
#define cE1 4
#define cF1 5
#define cG1 6
#define cH1 7

#define cA2 8
#define cB2 9
#define cC2 10
#define cD2 11
#define cE2 12
#define cF2 13
#define cG2 14
#define cH2 15

#define cA3 16
#define cB3 17
#define cC3 18
#define cD3 19
#define cE3 20
#define cF3 21
#define cG3 22
#define cH3 23

#define cA4 24
#define cB4 25
#define cC4 26
#define cD4 27
#define cE4 28
#define cF4 29
#define cG4 30
#define cH4 31

#define cA5 32
#define cB5 33
#define cC5 34
#define cD5 35
#define cE5 36
#define cF5 37
#define cG5 38
#define cH5 39

#define cA6 40
#define cB6 41
#define cC6 42
#define cD6 43
#define cE6 44
#define cF6 45
#define cG6 46
#define cH6 47

#define cA7 48
#define cB7 49
#define cC7 50
#define cD7 51
#define cE7 52
#define cF7 53
#define cG7 54
#define cH7 55

#define cA8 56
#define cB8 57
#define cC8 58
#define cD8 59
#define cE8 60
#define cF8 61
#define cG8 62
#define cH8 63

#define MAX_MOVIMENTS 240
#define MAX_SEGONESJUGADES 50

#define SeeDesconegut -9999

//http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c
#define BIT_SET(p, n) (p[(n) / CHAR_BIT] |=  (0x80>>((n) % CHAR_BIT ))); n++
#define BIT_CLEAR(p, n) (p[(n) / CHAR_BIT] &= ~(0x80>>((n) % CHAR_BIT ))); n++
#define BIT_ISSET(p, n) (p[(n) / CHAR_BIT] &   (0x80>>((n) % CHAR_BIT )))

#define mascarapassatsdebils 0xFFFFFFFFFFFF00

#endif