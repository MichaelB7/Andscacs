#ifndef INCLOS_MOVIMENTS_H
#define INCLOS_MOVIMENTS_H

#include <stdint.h>
#include "definicions.h"

/* bits moviment
0-5 = origen
6-11 = destí
12-15 =
0  0000 res
1  0001 enroc
2  0010 al pas
4  0100 corona caball
6  0110 corona alfil
10 1010 corona torre
12 1100 corona dama
*/

typedef uint16_t Moviment;

#define flag_enroc (1<<12)
#define flag_peo_al_pas (2<<12)
#define flag_corona_dama (Dama<<12)
#define flag_corona_torre (Torre<<12)
#define flag_corona_caball (Caball<<12)
#define flag_corona_alfil (Alfil<<12)

#define FerMoviment(origen,desti) (((desti) << 6) | (origen))
#define Origen(m) ((m) & 0x3f)
#define Desti(m) (((m) >> 6) & 0x3f)
#define EsPromocio(m) ((m) & 0xC000)
#define EsAlPas(m) (((m) & 0xF000) == flag_peo_al_pas)
#define EsEnroc(m) ((m) & flag_enroc)
#define EsEspecial(m) ((m) & 0xF000)
#define PecaPromocio(m,color) ((((m) >> 12) & 0xE) + (color))
#define JugadaInvalida 0
#define JugadaInvalida2 0xFFFF

struct MovList {
	int moviment;	//en comptes de 16 bits perquè vagi més ràpid
	int puntuacio;
};

void marcar_captures_bones_q(struct tss * RESTRICT ss);
void generar_moviments(tss * RESTRICT ss);
template<e_colors jo>
void generar_captures_i_coronacions(tss * RESTRICT ss);
template<e_colors jo>
void generar_quiets(tss * RESTRICT ss);
template<e_colors jo>
void generar_evasions(struct tss * RESTRICT ss);
void generar_possibles_escacs(struct tss * RESTRICT ss);
void generar_cc(struct tss * RESTRICT ss);
void fer_moviment(struct tss * RESTRICT ss, int m);
void fer_moviment_null(struct tss * RESTRICT ss);
void desfer_moviment(struct tss * RESTRICT ss, int m);
void desfer_moviment_null(tss * RESTRICT ss);
bool es_escac_rapid(tss * RESTRICT ss, int m);
bool es_escac_bo(tss * RESTRICT ss, int m);
Moviment moviment_ab(struct tss * RESTRICT ss);
bool jugada_ilegal(struct tss * RESTRICT ss, int m);
bool jugada_legal(struct tss * RESTRICT ss, int moviment, e_colors jo);
Moviment seguent_moviment(struct Estat_Analisis *estat_actual);

#define BonusMourePecaAtacadaPeo 1

enum etapes {
	etapa_res, etapa_bones_captures, etapa_evasions, etapa_segonmovimenthash, etapa_nullmovethreat, etapa_killer1, etapa_killer2, etapa_killer3, etapa_killer4, /*etapa_killer5,*/
	etapa_countermove1, etapa_countermove2, etapa_hashpeces1, etapa_hashpeces2, etapa_quiet, etapa_captures_dolentes, etapa_fi
};
#define valorcapturesdolentes -2000
#endif