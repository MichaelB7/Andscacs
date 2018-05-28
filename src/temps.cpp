#include "utils.h"
//necessari per linux
#undef min
#undef max

#include "temps.h"
#ifdef WINDOWS
#include <Windows.h>
#endif
#include <stdio.h>
#include <sys/timeb.h>
#include <stdint.h>
#include "analisis.h"
#include "debug.h"
#define _max(a,b)            (((a) > (b)) ? (a) : (b))  
#define _min(a,b)            (((a) < (b)) ? (a) : (b))

bool sortir_programa = false;
unsigned char temps_tipus;
int temps_temps[2];  //blanques/negres
int temps_inc[2]; //blanques/negres
int temps_NumMoviments;
int temps_Profunditat;
int temps_nodes;
int temps_mate;
int64_t temps_TempsMoviment;
enum tipus_tasques tasca;
long long inici_analisis, hora_actual;
#ifdef WINDOWS
LARGE_INTEGER frequency;        // ticks per second
#endif

int buffer_de_temps;//guardar una mica de temps de més, per si de cas
int movimentsperfer;  //número de moviments que es considera que queden fins el final de la partida.

//dades que s'apliquen a la partida
int64_t temps_per_moure;
int64_t temps_maxim;
int milisegons_entre_escoltar;

void calcular_temps_moviment() {
	int movimentsperferx;
	milisegons_entre_escoltar = 50;

	if (tasca != tasca_ponderar && (temps_tipus & (TipusInfinit | TipusProfunditat | TipusNodes))) {
		milisegons_entre_escoltar = 100;
		return;
	}

	if (temps_tipus & TipusTempsMoviment) {
		temps_per_moure = temps_TempsMoviment;
		return;
	}

	temps_per_moure = 0;
	movimentsperferx = movimentsperfer;
	if (temps_tipus & TipusNumMoviments)
		movimentsperferx = temps_NumMoviments + 2;  //si és un tipus de partida amb control, afegir dues jugades a les jugades que queden pel control per tenir una mica de buffer

	if (temps_temps[color_que_juga_pc] < 0)
		temps_temps[color_que_juga_pc] = 0;
	if (temps_inc[color_que_juga_pc]  < 0)
		temps_inc[color_que_juga_pc] = 0;

	temps_maxim = 0;
	int tempsopo = temps_temps[oponent(color_que_juga_pc)];
	if (tempsopo < 0)
		tempsopo = 0;
	if (temps_tipus & TipusTemps) {
		int tempsx = temps_temps[color_que_juga_pc] - (4 * resolucio_rellotge); //per deixar un mínim

		if (ssbase[0].moviment_a_ponderar != darrer_moviment)
			movimentsperferx -= 1;  //pensar una mica més si no han fet la jugada que esperava

		temps_per_moure += tempsx / movimentsperferx;
		temps_maxim = tempsx / _min(14, movimentsperferx);
		//si tenim més temps que el rival, podem pensar una mica més
		if (temps_temps[color_que_juga_pc] > tempsopo) {
			int j = (temps_temps[color_que_juga_pc] - tempsopo) >> 3;
			temps_per_moure += j;
			temps_maxim += j;
		}
		if (temps_tipus & TipusIncrement) {
			temps_per_moure += temps_inc[color_que_juga_pc];
			temps_maxim += temps_inc[color_que_juga_pc];
			if (temps_temps[color_que_juga_pc] < temps_inc[color_que_juga_pc] * 3) {
				temps_per_moure = temps_per_moure >> 1;
				temps_maxim = temps_maxim >> 1;
			}
		}
	}
	else
		if (temps_tipus & TipusIncrement) {
			temps_per_moure += temps_inc[color_que_juga_pc];
			temps_maxim += temps_inc[color_que_juga_pc];
		}
	temps_maxim -= 2 * resolucio_rellotge;
	if (temps_maxim <0)
		temps_maxim = temps_per_moure;

	if (temps_per_moure > 200)
		temps_per_moure -= 100;

	if (temps_per_moure>10000)
		milisegons_entre_escoltar = 100;
	else
		if (temps_per_moure<1000) {
			milisegons_entre_escoltar = temps_per_moure / (resolucio_rellotge * 3);
			if (milisegons_entre_escoltar<resolucio_rellotge)
				milisegons_entre_escoltar = resolucio_rellotge;
		}
	return;
}
