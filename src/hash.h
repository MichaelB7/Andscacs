#ifndef INCLOS_HASH_H
#define INCLOS_HASH_H

#include <stdint.h>
#include "analisis.h"
#include "debug.h"

#define tt_alpha 0
#define tt_beta 1
#define tt_exacte 2
#define tt_res 8
#define NumEntradesHashGrup 4
#define tt_sensedepth -127

extern uint32_t numaleatori;

#define CACHE_LINE_SIZE 64

typedef uint64_t ClauHash;	//també està definit a udefinicions.h
typedef uint32_t ClauGuardaHash;	//també està definit a udefinicions.h
typedef uint64_t ComptadorsHash;

struct thash{
	uint64_t w1;
	/*ClauGuardaHash clauhash;
	Moviment millor_moviment;
	Valor valor; //avaluació 	*/
	Valor score;
	Moviment segonmoviment;
	uint8_t tipuspuntuaciohash;  //tt_alpha,beta,exacte
	uint8_t edat;
	int8_t depth;
};

#define movimenthash(h) ((h.w1 >> 32) & 0xFFFF)
#define movimenthash_p(h) ((h->w1 >> 32) & 0xFFFF)
#define valorhash(h) ((h.w1 >> 48) & 0xFFFF)

#define mbhashdefecte 128 << 20;
extern ComptadorsHash memoriahashbytes; 
extern ComptadorsHash numentradeshash;
extern uint32_t HashMask;

extern struct thash *hash_;
extern void* memhash;
extern uint8_t edathash;
extern bool NeverClearHash;
extern char HashFile[1024];

inline thash* primera_entrada(const ClauHash key) {

	return hash_ + ((uint64_t)key & HashMask);
}

extern ClauHash ClauInici; //per no començar a 0
extern ClauHash ClauTorn;
extern ClauHash ClauPeca[16][64]; //fins el número del rei negre, que és el més gran
extern ClauHash ClauEnroc[16]; //combinacions de 4 bits
extern ClauHash ClauEP[8]; //només s'usen 16
extern ClauHash ClauMovimentExclos; //s'usa per diferenciar els hash guardats dels moviments exclosos com a conseqüència del Singular extension search

void inicialitza_claus_hash(uint32_t rands);
void posar_hash_posicio_i_valor_peces(tss *ss);

void guardar_hash(int score, int n, int nivell, ClauHash hashposicio, uint8_t tt_flag, Moviment moviment, Moviment segonmoviment, Valor avaluacio);
bool write_hash_binary();
void read_hash_binary();
void inicialitza_hash();
void buidar_hash();
struct thash* buscar_hash(ClauHash hashposicio);

//////////////
//Hash peons//
//////////////
#define MBHashPeons 4
#define HashPeonsFlagReiBlancPerill2a 1 //rei 2a fila i no té peo al costat llarg
#define HashPeonsFlagReiNegrePerill2a 2
#define HashPeonsFlagCentreBloquejat 4
#define HashPeonsFlagPenalitzacioSemiBloqueigCentralBlanques 8
#define HashPeonsFlagPenalitzacioSemiBloqueigCentralNegres 16
#define HashPeonsFlagBlanquesCandidataPassat 32
#define HashPeonsFlagNegresCandidataPassat 64

struct tHashPeons{
	ClauHash hash;
	Bitboard Passats, Debils, pawnAttacksSpan[2]; //a passats, s'aprofita 1r i darrer byte per PeonsDavant
	int32_t score; //no pot ser 16 bits, doncs té mig joc i final
	int8_t SeguretatRei[2];
	uint8_t flags; 					
};
extern tHashPeons HashPeons[(MBHashPeons << 20) / sizeof(tHashPeons)];
extern int NumHashPeons;
#endif