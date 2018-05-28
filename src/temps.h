#ifndef INCLUDED_TEMPS_H
#define INCLUDED_TEMPS_H

#include "debug.h"
#include <stdint.h>
#ifdef WINDOWS
#include <Windows.h>
#endif

#include <stdio.h>
#include <sys/timeb.h>
#ifdef LINUX
//necessari per linux
#undef min
#undef max
#include <chrono>
#endif

extern bool sortir_programa;
extern unsigned char temps_tipus;

enum TipusControl {
    TipusTemps=1,	// rellotge normal
    TipusIncrement=2,		// hi ha increment
    TipusNumMoviments=4,  //hi ha moviments pel control
    TipusProfunditat=8,	//mirar només x plies
    TipusNodes=16,	//buscar només x nodes
    TipusTempsMoviment=32,	//pensar exàctament durant x mseconds
    TipusInfinit=64	//fins stop
};

extern int temps_temps[2];  //blanques/negres
extern int temps_inc[2]; //blanques/negres
extern int temps_NumMoviments;
extern int temps_Profunditat;
extern int temps_nodes;
extern int64_t temps_TempsMoviment;
#ifdef WINDOWS
extern LARGE_INTEGER frequency;        // ticks per second
#endif

extern int buffer_de_temps;//guardar una mica de temps de més, per si de cas  //vell=500
extern int movimentsperfer;  //número de moviments que es considera que queden fins el final de la partida.

//dades que s'apliquen a la partida
extern int64_t temps_per_moure;
extern int64_t temps_maxim;
extern int milisegons_entre_escoltar;
#define resolucio_rellotge 5

void calcular_temps_moviment();

inline long long temps_actual() {
#ifdef WINDOWS
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return t.QuadPart * 1000.0 / frequency.QuadPart;
#endif
#ifdef LINUX
	return std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::steady_clock::now().time_since_epoch()).count();
#endif
}

enum tipus_tasques {
    tasca_cap,
    tasca_analitzar,
    tasca_ponderar
};

extern enum tipus_tasques tasca;
extern long long inici_analisis, hora_actual;
#endif