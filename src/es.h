#ifndef INCLOS_ES_H
#define INCLOS_ES_H
#include "debug.h"
#include "moviments.h"
#ifdef WINDOWS
#include "windows.h"
#endif

extern int Consola;
#ifdef WINDOWS
extern HANDLE HandleEntrada; 
#endif

void prepara_entrada();
int entrada_pendent();
void comandes_uci(int fertest);
int mostrar_moviment(Moviment m, Moviment pond);
void comanda_go(char * command);

#define jugadesguardades 8
struct tcje {
	Bitboard peonsblanques, peonsnegres;
	uint8_t jugades[jugadesguardades * 10 / 8];  //3 bits tipus peça (e_numpecescurts), 6 bits casella destí, fins 8 jugades, 72 bits/8 = 9 bytes
};

#endif