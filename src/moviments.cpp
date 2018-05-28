#include "moviments.h"
#include "analisis.h"
#include "utils.h"
#include "hash.h"
#include "debug.h"
#include "definicions.h"
#include "avaluacio.h"
#ifdef LINUX
#include <string.h>
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

inline MovList * peons_moviments_coronacions(Bitboard bb, MovList * RESTRICT ml, int increment) {
	Bitboard b;
	Moviment m;
	for (b = bb; b != 0; b &= b - 1) {
		uint8_t a = lsb(b);
		m = FerMoviment(a, a + increment);
		(ml++)->moviment = m | flag_corona_dama;
		(ml++)->moviment = m | flag_corona_caball;
		(ml++)->moviment = m | flag_corona_torre;
		(ml++)->moviment = m | flag_corona_alfil;
	}
	return ml;
}

MovList * peons_moviments_potser_captures(MovList * RESTRICT ml, Bitboard b, int casellapeoalpas, int increment) {
	if (b == 0)
		return ml;
	//1r mirar si peo al pas
	if (casellapeoalpas && (b & Bit(casellapeoalpas - increment))) {
		ml->moviment = FerMoviment(casellapeoalpas - increment, casellapeoalpas) | flag_peo_al_pas;
		ml++;
		b ^= Bit(casellapeoalpas - increment);
	}
altremovpeo:
	if (b == 0)
		return ml;
	uint8_t cas = lsb(b);
	uint8_t casd = cas + increment;
	ml->moviment = FerMoviment(cas, casd);
	ml++;
	b &= b - 1;
	goto altremovpeo;
}

MovList * peons_moviments_quiets_o_sense_peoalpas(MovList * RESTRICT ml, Bitboard b, int increment) {
	uint8_t cas;
	uint8_t casd;
	switch (popcount(b)) {
	case 8:
		cas = lsb(b);
		b &= b - 1;
		casd = cas + increment;
		ml->moviment = FerMoviment(cas, casd);
		ml++;
	case 7:
		cas = lsb(b);
		b &= b - 1;
		casd = cas + increment;
		ml->moviment = FerMoviment(cas, casd);
		ml++;
	case 6:
		cas = lsb(b);
		b &= b - 1;
		casd = cas + increment;
		ml->moviment = FerMoviment(cas, casd);
		ml++;
	case 5:
		cas = lsb(b);
		b &= b - 1;
		casd = cas + increment;
		ml->moviment = FerMoviment(cas, casd);
		ml++;
	case 4:
		cas = lsb(b);
		b &= b - 1;
		casd = cas + increment;
		ml->moviment = FerMoviment(cas, casd);
		ml++;
	case 3:
		cas = lsb(b);
		b &= b - 1;
		casd = cas + increment;
		ml->moviment = FerMoviment(cas, casd);
		ml++;
	case 2:
		cas = lsb(b);
		b &= b - 1;
		casd = cas + increment;
		ml->moviment = FerMoviment(cas, casd);
		ml++;
	case 1:
		cas = lsb(b);
		casd = cas + increment;
		ml->moviment = FerMoviment(cas, casd);
		ml++;
	case 0:
		return ml;
	default: //per posicions estranyes amb > 8 peons
	altremovpeo2 :
		if (b == 0)
			return ml;
				 cas = lsb(b);
				 casd = cas + increment;
				 ml->moviment = FerMoviment(cas, casd);
				 ml++;
				 b &= b - 1;
				 goto altremovpeo2;
	}
}

template<e_colors jo>
void generar_captures_i_coronacions(tss * RESTRICT ss) {
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	const int ell = oponent(jo);
	Bitboard contrari;
	Bitboard contrariambpeoalpas;

	if (estat_actual->nivell <= NivellRecaptures && !estat_actual->escac) {
		contrari = Bit(Desti(estat_actual->moviment_previ));
		contrariambpeoalpas = contrari;
	}
	else {
		contrari = ss->tau.peces[ell];
		contrariambpeoalpas = contrari | (estat_actual->casellapeoalpas ? Bit(estat_actual->casellapeoalpas) : 0);  //inclou peo al pas
	}
	const int increment_columna_anterior_abs = jo ? 9 : 7;
	const int increment_columna_posterior_abs = jo ? 7 : 9;
	const Bitboard filapeocorona = jo ? FILA2 : FILA7;

	//***************
	//captures de peo
	//***************

	//a la columna posterior
	//als meus peons treure la columna H
	//agafar peces del contrari i desplaçarles. Agafar les caselles on coincideixen amb els peons

	MovList * RESTRICT ml = ss->mov;

	Bitboard a = (Tpeons(jo) & (~COLUMNAH)) & (ShiftPerPeons(jo, contrariambpeoalpas, increment_columna_posterior_abs));
	//posar captures amb coronacions
	int increment = jo ? -increment_columna_posterior_abs : increment_columna_posterior_abs;
	ml = peons_moviments_coronacions((a & filapeocorona), ml, increment);
	//posar captures normals
	ml = peons_moviments_potser_captures(ml, (a & (~filapeocorona)), estat_actual->casellapeoalpas, increment);

	//a la columna anterior
	a = (Tpeons(jo) & (~COLUMNAA)) & (ShiftPerPeons(jo, contrariambpeoalpas, increment_columna_anterior_abs));
	//posar captures amb coronacions
	increment = jo ? -increment_columna_anterior_abs : increment_columna_anterior_abs;
	ml = peons_moviments_coronacions((a & filapeocorona), ml, increment);
	//posar captures normals
	ml = peons_moviments_potser_captures(ml, (a & (~filapeocorona)), estat_actual->casellapeoalpas, increment);

	//posar coronacions sense captura
	//Els peons que estan per coronar
	//els que davant no tenen cap peça
	a = (Tpeons(jo) & filapeocorona) & (~ShiftPerPeons(jo, TTotespeces, 8));
	increment = Avancar(jo); //jo ? -8 : 8;
	ml = peons_moviments_coronacions(a, ml, increment);

	//*********************
	//captures altres peces
	//*********************

	/*for (a = AtacsRei[lsb(Trei(jo))] & contrari & (~estat_actual->atacs[ell]); a != 0; a &= a - 1)
	(ss->mov++)->moviment = FerMoviment(lsb(Trei(jo)), lsb(a));*/

	uint8_t casrei = lsb(Trei(jo));
	a = AtacsRei[casrei] & contrari & (~estat_actual->atacs[ell]);
	if (a == 0)
		goto movcaballs;
altremovrei:
	uint8_t desti; desti = lsb(a);
	ml->moviment = FerMoviment(casrei, desti);
	ml++;
	a &= a - 1;
	if (a) goto altremovrei;
movcaballs:
	/*for (a = Tcaballs(jo); a != 0; a &= a - 1)
	for (b = AtacsCaball[lsb(a)] & contrari; b != 0; b &= b - 1)
	(ss->mov++)->moviment = FerMoviment(lsb(a), lsb(b));*/
	a = Tcaballs(jo);
	Bitboard b;
altrecaball:
	if (a == 0)
		goto movalfils;
	uint8_t origen; origen = lsb(a);
	b = AtacsCaball[origen] & contrari;
altrecasellacaball:
	if (b == 0) {
		a &= a - 1;
		goto altrecaball;
	}
	desti = lsb(b);
	ml->moviment = FerMoviment(origen, desti);
	ml++;
	b &= b - 1;
	goto altrecasellacaball;

movalfils:

	a = Talfils(jo);
	int n = 0;
altrealfil:
	if (a == 0)
		goto movtorres;
	origen = lsb(a);
	b = estat_actual->atacsalfil[jo][n] & contrari;
altrecasellaalfil:
	if (b == 0) {
		a &= a - 1;
		n++;
		goto altrealfil;
	}
	desti = lsb(b);
	ml->moviment = FerMoviment(origen, desti);
	ml++;
	b &= b - 1;
	goto altrecasellaalfil;

movtorres:

	a = Ttorres(jo);
	n = 0;
altretorre:
	if (a == 0)
		goto movdames;
	origen = lsb(a);
	b = estat_actual->atacstorre[jo][n] & contrari;
altrecasellatorre:
	if (b == 0) {
		a &= a - 1;
		n++;
		goto altretorre;
	}
	desti = lsb(b);
	ml->moviment = FerMoviment(origen, desti);
	ml++;
	b &= b - 1;
	goto altrecasellatorre;

movdames:

	a = Tdames(jo);
	n = 0;
altredama:
	if (a == 0) {
		ss->mov = ml;
		return;
	}
	origen = lsb(a);
	b = estat_actual->atacsdama[jo][n] & contrari;
altrecaselladama:
	if (b == 0) {
		a &= a - 1;
		n++;
		goto altredama;
	}
	desti = lsb(b);
	ml->moviment = FerMoviment(origen, desti);
	ml++;
	b &= b - 1;
	goto altrecaselladama;
}

template<e_colors jo>
void generar_quiets(tss * RESTRICT ss) {
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	int n;
	const int ell = oponent(jo);
	const Bitboard filapeocorona = jo ? FILA2 : FILA7;
	const Bitboard segonafila = jo ? FILA7 : FILA2;
	const Bitboard ocupat = TTotespeces;
	const Bitboard lliure = ~ocupat;

	MovList * RESTRICT ml = ss->mov;

	//enroc
	if (jo == blanques) {
		if ((estat_actual->enroc & PotEnrocarBlancCurt) && !(ocupat & 0x60) && !(estat_actual->atacs[negres] & 0x70))
			(ml++)->moviment = FerMoviment(4, 6) | flag_enroc;
		if ((estat_actual->enroc & PotEnrocarBlancLlarg) && !(ocupat & 0x0E) && !(estat_actual->atacs[negres] & 0x1C))
			(ml++)->moviment = FerMoviment(4, 2) | flag_enroc;
	}
	else {
		if ((estat_actual->enroc & PotEnrocarNegreCurt) && !(ocupat & 0x6000000000000000) && !(estat_actual->atacs[blanques] & 0x7000000000000000))
			(ml++)->moviment = FerMoviment(60, 62) | flag_enroc;
		if ((estat_actual->enroc & PotEnrocarNegreLlarg) && !(ocupat & 0x0E00000000000000) && !(estat_actual->atacs[blanques] & 0x1C00000000000000))
			(ml++)->moviment = FerMoviment(60, 58) | flag_enroc;
	}

	//avanç peons, que no coronin
	Bitboard a = (Tpeons(jo) & ~filapeocorona) & (ShiftPerPeons(jo, lliure, 8));
	int increment = Avancar(jo); //jo ? -8 : 8;
	ml = peons_moviments_quiets_o_sense_peoalpas(ml, a, increment);

	//avanç 2 caselles
	a = (a & segonafila) & (ShiftPerPeons(jo, lliure, 16));
	increment = increment << 1;
	ml = peons_moviments_quiets_o_sense_peoalpas(ml, a, increment);

	/*for (a = Tcaballs(jo); a != 0; a &= a - 1)
	for (b = AtacsCaball[lsb(a)] & lliure; b != 0; b &= b - 1)
	(ss->mov++)->moviment = FerMoviment(lsb(a), lsb(b));*/
	a = Tcaballs(jo);
	Bitboard b;
altrecaball:
	if (a == 0)
		goto movalfils;
	uint8_t origen; origen = lsb(a);
	b = AtacsCaball[origen] & lliure;
altrecasellacaball:
	if (b == 0) {
		a &= a - 1;
		goto altrecaball;
	}
	uint8_t desti; desti = lsb(b);
	ml->moviment = FerMoviment(origen, desti);
	ml++;
	b &= b - 1;
	goto altrecasellacaball;

movalfils:

	a = Talfils(jo);
	n = 0;
altrealfil:
	if (a == 0)
		goto movtorres;
	origen = lsb(a);
	b = estat_actual->atacsalfil[jo][n] & lliure;
altrecasellaalfil:
	if (b == 0) {
		a &= a - 1;
		n++;
		goto altrealfil;
	}
	desti = lsb(b);
	ml->moviment = FerMoviment(origen, desti);
	ml++;
	b &= b - 1;
	goto altrecasellaalfil;

movtorres:

	a = Ttorres(jo);
	n = 0;
altretorre:
	if (a == 0)
		goto movdames;
	origen = lsb(a);
	b = estat_actual->atacstorre[jo][n] & lliure;
altrecasellatorre:
	if (b == 0) {
		a &= a - 1;
		n++;
		goto altretorre;
	}
	desti = lsb(b);
	ml->moviment = FerMoviment(origen, desti);
	ml++;
	b &= b - 1;
	goto altrecasellatorre;

movdames:

	a = Tdames(jo);
	n = 0;
altredama:
	if (a == 0)
		goto movrei;
	origen = lsb(a);
	b = estat_actual->atacsdama[jo][n] & lliure;
altrecaselladama:
	if (b == 0) {
		a &= a - 1;
		n++;
		goto altredama;
	}
	desti = lsb(b);
	ml->moviment = FerMoviment(origen, desti);
	ml++;
	b &= b - 1;
	goto altrecaselladama;

movrei:
	uint8_t casrei = lsb(Trei(jo));
	a = AtacsRei[casrei] & lliure & (~estat_actual->atacs[ell]);
altremovrei:
	if (a == 0) {
		ss->mov = ml;
		return;
	}
	desti = lsb(a);
	ml->moviment = FerMoviment(casrei, desti);
	ml++;
	a &= a - 1;
	goto altremovrei;
}

inline MovList * peons_moviments_escac(MovList * RESTRICT ml, Bitboard bb, Bitboard casellespotescac, int increment) {
altremovpeo2:
	if (bb == 0)
		return ml;
	uint8_t cas = lsb(bb);
	uint8_t casd = cas + increment;
	if ((Bit(cas) | Bit(casd)) & casellespotescac) {
		ml->moviment = FerMoviment(cas, casd);
		ml++;
	}
	bb &= bb - 1;
	goto altremovpeo2;
}

//no genera escacs amb enroc ni amb captures
//genera també descoberts
template<e_colors jo> void generar_possibles_escacs2(struct tss * RESTRICT ss, Estat_Analisis * RESTRICT estat_actual) {
	Bitboard a, b;
	int n;
	const int ell = oponent(jo);
	const Bitboard filapeocorona = jo ? FILA2 : FILA7;
	const Bitboard segonafila = jo ? FILA7 : FILA2;
	const Bitboard ocupat = TTotespeces;
	const Bitboard lliure = ~ocupat;
	const int posrei = lsb(Trei(ell));

	MovList * RESTRICT ml = ss->mov;

	if (ss->estat_actual->descoberts != 0)
		ss->estat_actual->descoberts = ss->estat_actual->descoberts;
	Bitboard casellespotescac = ((AtacsTorre(posrei, TTotespeces) | AtacsAlfil(posrei, TTotespeces)) & ~TTotespeces) | ss->estat_actual->descoberts;
	if (casellespotescac < 2 || casellespotescac == 9223372036854775808 || casellespotescac == 72057594037927936 || casellespotescac == 128)  //si rei cobert a l'enroc, no és possible fer escac amb sliders sense captures
		goto sipotambcaball;

	//avanç peons, que no coronin
	a = (Tpeons(jo) & ~filapeocorona) & (ShiftPerPeons(jo, lliure, 8));
	int increment; increment = Avancar(jo); //jo ? -8 : 8;
	ml = peons_moviments_escac(ml, a, casellespotescac, increment);

	//avanç 2 caselles
	a = (a & segonafila) & (ShiftPerPeons(jo, lliure, 16));
	increment = increment << 1;
	ml = peons_moviments_escac(ml, a, casellespotescac, increment);

	Bitboard c, d;
	for (a = Talfils(jo), n = 0; a != 0; a ^= d, n++) {
		d = a & ~(a - 1);
		uint8_t z1; z1 = lsb(a);
		for (b = estat_actual->atacsalfil[jo][n] & lliure; b != 0; b ^= c) {
			c = b & ~(b - 1);
			if ((d | c) & casellespotescac)
				(ml++)->moviment = FerMoviment(z1, lsb(b));
		}
	}

	for (a = Ttorres(jo), n = 0; a != 0; a ^= d, n++) {
		d = a & ~(a - 1);
		uint8_t z1; z1 = lsb(a);
		for (b = estat_actual->atacstorre[jo][n] & lliure; b != 0; b ^= c) {
			c = b & ~(b - 1);
			if ((d | c) & casellespotescac)
				(ml++)->moviment = FerMoviment(z1, lsb(b));
		}
	}

	for (a = Tdames(jo), n = 0; a != 0; a ^= d, n++) {
		d = a & ~(a - 1);
		uint8_t z1; z1 = lsb(a);
		for (b = estat_actual->atacsdama[jo][n] & lliure; b != 0; b ^= c) {
			c = b & ~(b - 1);
			if ((d | c) & casellespotescac)
				(ml++)->moviment = FerMoviment(z1, lsb(b));
		}
	}

	for (a = AtacsRei[lsb(Trei(jo))] & lliure & (~estat_actual->atacs[ell]); a != 0; a &= a - 1) {
		uint8_t z1; z1 = lsb(a);
		uint8_t z2; z2 = lsb(Trei(jo));
		if ((Bit(z1) | Bit(z2)) & casellespotescac)
			(ml++)->moviment = FerMoviment(z2, z1);
	}

sipotambcaball:
	casellespotescac = AtacsCaball[posrei] & ~ss->tau.peces[ell];
	for (a = Tcaballs(jo); a != 0; a ^= d) {
		d = a & ~(a - 1);
		uint8_t z1; z1 = lsb(a);
		for (b = AtacsCaball[z1] & lliure; b != 0; b ^= c) {
			c = b & ~(b - 1);
			if ((d | c) & casellespotescac)
				(ml++)->moviment = FerMoviment(z1, lsb(b));
		}
	}
	ss->mov = ml;
}

//aquesta funció cal perquè sino no hi ha manera de compilar no sé perquè
void generar_possibles_escacs(struct tss * RESTRICT ss) {
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	if (estat_actual->mou == blanques)
		generar_possibles_escacs2<blanques>(ss, estat_actual);
	else
		generar_possibles_escacs2<negres>(ss, estat_actual);
}

void generar_moviments(tss * RESTRICT ss) {
	if (ss->estat_actual->mou == blanques) {
		generar_captures_i_coronacions<blanques>(ss);
		generar_quiets<blanques>(ss);
	}
	else {
		generar_captures_i_coronacions<negres>(ss);
		generar_quiets<negres>(ss);
	}
	ss->mov->moviment = JugadaInvalida;
}

void generar_cc(tss * RESTRICT ss) {
	if (ss->estat_actual->mou == blanques)
		generar_captures_i_coronacions<blanques>(ss);
	else
		generar_captures_i_coronacions<negres>(ss);
	ss->mov->moviment = JugadaInvalida;
}

template<e_colors jo>
void generar_evasions(tss * RESTRICT ss) {
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	Bitboard a, b;
	int n;
	const int ell = oponent(jo);
	MovList * RESTRICT ml = ss->mov;

	n = lsb(Trei(jo));
	for (a = AtacsRei[n] & (~ss->tau.peces[jo]) & (~estat_actual->atacs[ell]) & (~estat_actual->tampocpotanarrei); a != 0; a &= a - 1)
		(ml++)->moviment = FerMoviment(n, lsb(a));

	if (estat_actual->destinsescac == 0) //escac doble
		goto figenev;

	//***************
	//captures de peo
	//***************
	Bitboard contrari; contrari = ss->tau.peces[ell];
	int increment_columna_anterior_abs; increment_columna_anterior_abs = jo ? 9 : 7;
	int increment_columna_posterior_abs; increment_columna_posterior_abs = jo ? 7 : 9;
	Bitboard filapeocorona; filapeocorona = jo ? FILA2 : FILA7;
	Bitboard lliure; lliure = ~TTotespeces;
	Bitboard segonafila; segonafila = jo ? FILA7 : FILA2;

	//a la columna posterior
	//als meus peons treure la columna H
	//agafar peces del contrari i desplaçarles. Agafar les caselles on coincideixen amb els peons
	a = (Tpeons(jo) & (~COLUMNAH)) & (ShiftPerPeons(jo, estat_actual->destinsescac & contrari, increment_columna_posterior_abs));
	//posar captures amb coronacions
	int increment; increment = jo ? -increment_columna_posterior_abs : increment_columna_posterior_abs;
	ml = peons_moviments_coronacions((a & filapeocorona), ml, increment);
	//posar captures normals
	ml = peons_moviments_quiets_o_sense_peoalpas(ml, (a & (~filapeocorona)), increment);

	//a la columna anterior
	a = (Tpeons(jo) & (~COLUMNAA)) & (ShiftPerPeons(jo, estat_actual->destinsescac & contrari, increment_columna_anterior_abs));
	//posar captures amb coronacions
	increment = jo ? -increment_columna_anterior_abs : increment_columna_anterior_abs;
	ml = peons_moviments_coronacions((a & filapeocorona), ml, increment);
	//posar captures normals
	ml = peons_moviments_quiets_o_sense_peoalpas(ml, (a & (~filapeocorona)), increment);

	//posar coronacions sense captura
	//Els peons que estan per coronar
	//els que davant no tenen cap peça			i coronant tapen l'escac
	a = (Tpeons(jo) & filapeocorona) & (~ShiftPerPeons(jo, TTotespeces, 8)) & (ShiftPerPeons(jo, estat_actual->destinsescac, 8));
	increment = Avancar(jo); //jo ? -8 : 8;
	ml = peons_moviments_coronacions(a, ml, increment);

	//avanç peons, que no coronin
	a = (Tpeons(jo) & ~filapeocorona) & (ShiftPerPeons(jo, lliure & estat_actual->destinsescac, 8));
	ml = peons_moviments_quiets_o_sense_peoalpas(ml, a, increment);

	//avanç 2 caselles
	a = (Tpeons(jo) & segonafila) & (ShiftPerPeons(jo, lliure & estat_actual->destinsescac, 16)) & (ShiftPerPeons(jo, lliure, 8));
	increment = increment << 1;
	ml = peons_moviments_quiets_o_sense_peoalpas(ml, a, increment);

	//parar escac amb peo al pas
	if (estat_actual->casellapeoalpas) {
		if (Bit(estat_actual->casellapeoalpas ^ 8) == estat_actual->destinsescac) {
			if (COL(estat_actual->casellapeoalpas) > 0 && (Tpeons(jo) & Bit((estat_actual->casellapeoalpas ^ 8) - 1)))
				(ml++)->moviment = FerMoviment((estat_actual->casellapeoalpas ^ 8) - 1, estat_actual->casellapeoalpas) | flag_peo_al_pas;
			if (COL(estat_actual->casellapeoalpas) < 7 && (Tpeons(jo) & Bit((estat_actual->casellapeoalpas ^ 8) + 1)))
				(ml++)->moviment = FerMoviment((estat_actual->casellapeoalpas ^ 8) + 1, estat_actual->casellapeoalpas) | flag_peo_al_pas;
		}
	}

	for (a = Tcaballs(jo); a != 0; a &= a - 1)
		for (b = AtacsCaball[lsb(a)] & estat_actual->destinsescac; b != 0; b &= b - 1)
			(ml++)->moviment = FerMoviment(lsb(a), lsb(b));

	for (a = Talfils(jo), n = 0; a != 0; a &= a - 1, n++)
		for (b = estat_actual->atacsalfil[jo][n] & estat_actual->destinsescac; b != 0; b &= b - 1)
			(ml++)->moviment = FerMoviment(lsb(a), lsb(b));

	for (a = Ttorres(jo), n = 0; a != 0; a &= a - 1, n++)
		for (b = estat_actual->atacstorre[jo][n] & estat_actual->destinsescac; b != 0; b &= b - 1)
			(ml++)->moviment = FerMoviment(lsb(a), lsb(b));

	for (a = Tdames(jo), n = 0; a != 0; a &= a - 1, n++)
		for (b = estat_actual->atacsdama[jo][n] & estat_actual->destinsescac; b != 0; b &= b - 1)
			(ml++)->moviment = FerMoviment(lsb(a), lsb(b));
figenev:
	ml->moviment = JugadaInvalida;
	ss->mov = ml;
}

void fer_moviment(tss * RESTRICT ss, int m){

	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	ss->numjugadesfetes++;
	unsigned int origen = Origen(m);
	unsigned int desti = Desti(m);
	unsigned int mogut = ss->tau.c[origen];
	unsigned int menjat = ss->tau.c[desti];

	Estat_Analisis * RESTRICT seg = estat_actual + 1;
	seg->casellapeoalpas = CasellaBuida;
	seg->menjat = menjat;
	seg->mou = oponent(estat_actual->mou);
	seg->PliesDesdeNull = estat_actual->PliesDesdeNull + 1;
	seg->valorpeces[0] = estat_actual->valorpeces[0];
	seg->valorpeces[1] = estat_actual->valorpeces[1];
	memcpy(&seg->numpeces, &estat_actual->numpeces, sizeof(seg->numpeces));
	seg->calculaatacs = true;
	seg->enroc = estat_actual->enroc & FlagsEnroc[desti] & FlagsEnroc[origen];

	ss->tau.c[origen] = CasellaBuida;
	uint64_t bitorigen = Bit(origen);
	ss->tau.peces[mogut] ^= bitorigen;
	ss->tau.peces[estat_actual->mou] ^= bitorigen; //Totes les peces blanques o negres
	ss->tau.c[desti] = mogut;

	uint64_t bitdesti = Bit(desti);
	ss->tau.peces[mogut] |= bitdesti;
	ss->tau.peces[estat_actual->mou] |= bitdesti; //Totes les peces blanques o negres

	seg->hashpeons = estat_actual->hashpeons;
	seg->jugades50 = estat_actual->jugades50 + 1;
	seg->hash = estat_actual->hash;
	if (menjat) {
		seg->jugades50 = 0;
		ss->tau.peces[menjat] ^= bitdesti;
		ss->tau.peces[seg->mou] ^= bitdesti; //Totes les peces blanques o negres
		seg->hash ^= ClauPeca[menjat][desti];
		if (peca_sense_color(menjat) == Peo) {
			seg->hashpeons ^= ClauPeca[menjat][desti];
		}
		seg->numpeces[menjat] --;
		seg->valorpeces[seg->mou] -= valorpeca[menjat];
	}
	seg->hash ^= ClauPeca[mogut][origen] ^ ClauPeca[mogut][desti] ^ ClauEnroc[estat_actual->enroc] ^ ClauEnroc[seg->enroc];
	if (peca_sense_color(mogut) == Peo || peca_sense_color(mogut) == Rei)
		seg->hashpeons ^= ClauPeca[mogut][origen] ^ ClauPeca[mogut][desti];

	if (estat_actual->casellapeoalpas != CasellaBuida)
		seg->hash ^= ClauEP[COL(estat_actual->casellapeoalpas)];

	if (peca_sense_color(mogut) == Peo) {
		seg->jugades50 = 0;

		if (EsPromocio(m)) {
			ss->tau.peces[mogut] ^= bitdesti; //treu peo
			seg->valorpeces[estat_actual->mou] -= valorpeca[mogut];
			seg->numpeces[mogut] --; //treu peo
			mogut = PecaPromocio(m, estat_actual->mou);
			seg->numpeces[mogut]++; //afegeix peça coronada
			seg->valorpeces[estat_actual->mou] += valorpeca[mogut];
			ss->tau.peces[mogut] |= bitdesti; //afegeix peça coronada
			ss->tau.c[desti] = mogut;
			Bitboard x1 = ClauPeca[afegir_color_a_peca(Peo, estat_actual->mou)][desti];
			seg->hash ^= ClauPeca[mogut][desti] ^ x1;
			seg->hashpeons ^= x1;
			seg->atacpeoacabademoure = 0;
		}
		else {
			seg->atacpeoacabademoure = AtacsPeo[estat_actual->mou][desti];
			if (EsAlPas(m)) {
				int casellapeomenjat = desti ^ 8;
				ss->tau.c[casellapeomenjat] = CasellaBuida;
				uint64_t bitpeo = Bit(casellapeomenjat);
				menjat = afegir_color_a_peca(Peo, seg->mou);
				ss->tau.peces[menjat] ^= bitpeo;
				ss->tau.peces[seg->mou] ^= bitpeo; //Totes les peces blanques o negres
				seg->hash ^= ClauPeca[menjat][casellapeomenjat];
				seg->hashpeons ^= ClauPeca[menjat][casellapeomenjat];
				seg->numpeces[menjat] --;
				seg->valorpeces[seg->mou] -= valorpeca[menjat];
			}
			else if ((desti ^ origen) == 16) {
				//només marcar la casella de peo al pas si hi ha peons que puguin menjar al pas
				Bitboard CasellesOnHiHaDHaverPeo = 0;
				if (COL(desti) > 0)
					CasellesOnHiHaDHaverPeo = Bit(desti - 1);
				if (COL(desti) < 7)
					CasellesOnHiHaDHaverPeo |= Bit(desti + 1);
				if (ss->tau.peces[afegir_color_a_peca(Peo, seg->mou)] & CasellesOnHiHaDHaverPeo) {
					seg->casellapeoalpas = (desti + origen) >> 1;  // la casella del mig. p.e. a e2-e4, seria e3
					seg->hash ^= ClauEP[COL(seg->casellapeoalpas)];
				}
			}
		}
	}
	else {
		seg->atacpeoacabademoure = 0;
		if (EsEnroc(m)) {
			seg->jugades50 = 0;
			int torreo, torred;
			if (desti == 6) {
				torreo = 7;
				torred = 5;
			}
			else if (desti == 62) {
				torreo = 63;
				torred = 61;
			}
			else if (desti == 2) {
				torreo = 0;
				torred = 3;
			}
			else if (desti == 58) {
				torreo = 56;
				torred = 59;
			}
			uint8_t torre = afegir_color_a_peca(Torre, estat_actual->mou);
			ss->tau.c[torreo] = CasellaBuida;
			ss->tau.c[torred] = torre;
			ss->tau.peces[torre] ^= Bit(torreo);
			ss->tau.peces[torre] |= Bit(torred);
			ss->tau.peces[estat_actual->mou] ^= Bit(torreo);
			ss->tau.peces[estat_actual->mou] |= Bit(torred);
			seg->hash ^= ClauPeca[torre][torreo] ^ ClauPeca[torre][torred];
		}
	}

	seg->hash ^= ClauTorn;
#ifdef WINDOWS
	_mm_prefetch((char*)primera_entrada(seg->hash), _MM_HINT_T0);
#endif
#ifdef LINUX
	__builtin_prefetch(primera_entrada(seg->hash));
#endif
#ifdef WINDOWS
#endif
#ifdef LINUX
#endif

	ss->darrer_ply_historic++;
	ss->historic_posicions[ss->darrer_ply_historic] = seg->hash;
	seg->moviment_previ = m;
	seg->avaluacio = puntuacioinvalida;
}

void fer_moviment_null(tss * RESTRICT ss) {
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	ss->numjugadesfetes++;
	Estat_Analisis * RESTRICT seg = estat_actual + 1;
	seg->hash = estat_actual->hash ^ ClauTorn;
	seg->hashpeons = estat_actual->hashpeons;
	seg->calculaatacs = false;
	if (estat_actual->casellapeoalpas != CasellaBuida)
		seg->hash ^= ClauEP[COL(estat_actual->casellapeoalpas)];
#ifdef WINDOWS
	_mm_prefetch((char *)&hash_ + ((uint32_t)seg->hash & HashMask), _MM_HINT_T0);
#endif
#ifdef LINUX
	__builtin_prefetch(&hash_ + ((uint32_t)seg->hash & HashMask));
#endif

	seg->mou = oponent(estat_actual->mou);

	memcpy(&seg->atacs[0], &estat_actual->atacs[0], (&seg->clavats - &seg->atacs[0])*sizeof(Bitboard));

	seg->clavats = estat_actual->clavats;
	seg->descoberts = estat_actual->descoberts;
	seg->jugades50 = 0;
	seg->PliesDesdeNull = 0;
	seg->enroc = estat_actual->enroc;
	seg->casellapeoalpas = CasellaBuida;
	seg->menjat = CasellaBuida;
	seg->avaluacio = -estat_actual->avaluacio;
	seg->valorpeces[0] = estat_actual->valorpeces[0];
	seg->valorpeces[1] = estat_actual->valorpeces[1];
	seg->moviment_previ = JugadaInvalida;
	memcpy(&seg->numpeces, &estat_actual->numpeces, sizeof(seg->numpeces));
	seg->atacpeoacabademoure = 0;
	ss->darrer_ply_historic++;
	ss->historic_posicions[ss->darrer_ply_historic] = seg->hash;
}

void desfer_moviment(tss * RESTRICT ss, int m){
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	unsigned int desti = Desti(m);
	unsigned int mogut = ss->tau.c[desti];
	int ell = estat_actual->mou;
	int jo = oponent(estat_actual->mou);
	uint64_t bitdesti = Bit(desti);
	ss->tau.c[desti] = estat_actual->menjat;

	if (EsPromocio(m)) {
		ss->tau.peces[mogut] ^= bitdesti;
		mogut = afegir_color_a_peca(Peo, jo);
	}
	unsigned int origen = Origen(m);
	uint64_t bitorigen = Bit(origen);
	ss->tau.c[origen] = mogut;
	ss->tau.peces[mogut] |= bitorigen;
	ss->tau.peces[jo] |= bitorigen;

	ss->tau.peces[mogut] &= ~bitdesti;
	ss->tau.peces[jo] ^= bitdesti;
	if (estat_actual->menjat) {
		ss->tau.peces[estat_actual->menjat] |= bitdesti;
		ss->tau.peces[ell] |= bitdesti;
	}
	else {
		if (EsEnroc(m)) {
			int torreo, torred;
			if (desti == 6) {
				torreo = 7;
				torred = 5;
			}
			else if (desti == 62) {
				torreo = 63;
				torred = 61;
			}
			else if (desti == 2) {
				torreo = 0;
				torred = 3;
			}
			else if (desti == 58) {
				torreo = 56;
				torred = 59;
			}
			uint8_t torre = afegir_color_a_peca(Torre, jo);
			ss->tau.c[torred] = 0;
			ss->tau.c[torreo] = torre;
			ss->tau.peces[torre] ^= Bit(torred);
			ss->tau.peces[jo] ^= Bit(torred);
			ss->tau.peces[torre] |= Bit(torreo);
			ss->tau.peces[jo] |= Bit(torreo);
		}
		else if (EsAlPas(m)) {
			desti = desti ^ 8;
			bitdesti = Bit(desti);
			mogut = afegir_color_a_peca(Peo, ell);
			ss->tau.c[desti] = mogut;
			ss->tau.peces[mogut] |= bitdesti;
			ss->tau.peces[ell] |= bitdesti;
		}
	}
	ss->darrer_ply_historic--;
	ss->estat_actual--;
}


void desfer_moviment_null(tss * RESTRICT ss) {
	ss->darrer_ply_historic--;
}

//en una posició, si el moviment fa escac. no s'usa
bool es_escac_rapid(tss * RESTRICT ss, int m) { // no detecta escacs amb enroc ni al pas ni amb algunes coronacions
	uint64_t king;
	int from, to, piece, king_sq;
	int me = ss->estat_actual->mou;
	int opp = oponent(me);

	from = Origen(m);
	to = Desti(m);
	king = Trei(opp);
	king_sq = lsb(king);
	piece = ss->tau.c[from];
	if ((Bit(from) & ss->estat_actual->descoberts) //peça que pot fer escac descobert
		&& !(Direccio[king_sq][from] & Bit(to))) return true;  //escac descobert doncs es mou fora de la direcció d'escac descobert
	if (piece >= ReiBlanc)
		return 0;
	if (piece < CaballBlanc) {
		if (AtacsPeo[me][to] & king) return 1;
		if ((ROW(king) == ROW(to) && (ROW(to) == 0 || ROW(to) == 7)) && !(Entre[to][king_sq] & TTotespeces)) return 1;
		return 0;
	}
	else if (piece < TorreBlanca) { //caball o alfil
		Bitboard b = king | Bit(from);
		//casella origen ha de ser del mateix color que la casella del rei
		if (!((QuadresNegres | b) == QuadresNegres || (~QuadresNegres | b) == ~QuadresNegres))
			return 0;
		if (piece < AlfilBlanc) {
			if (AtacsCaball[to] & king) return 1;
			return 0;
		}
		else
			if (PrecalculAlfil[to] & king)
				if (!(Entre[king_sq][to] & TTotespeces))
					return 1;
		return 0;
	}
	//torre o dama
	if (ROW(king_sq) == ROW(to) || COL(king_sq) == COL(to)) {
		if (!(Entre[king_sq][to] & TTotespeces))
			return 1;
		return 0;
	}
	if (piece >= DamaBlanca) {
		if (PrecalculAlfil[to] & king) if (!(Entre[king_sq][to] & TTotespeces))
			return 1;
	}
	return 0;
}

bool es_escac_bo(tss * RESTRICT ss, int m) {
	uint64_t king;
	int from, to, piece, king_sq;
	int me = ss->estat_actual->mou;
	int opp = oponent(me);

	from = Origen(m);
	to = Desti(m);
	king = Trei(opp);
	king_sq = lsb(king);
	Bitboard hh = ss->estat_actual->descoberts;
	if (hh && (Bit(from) & hh) //peça que pot fer escac descobert
		&& !(Direccio[king_sq][from] & Bit(to))) return true;  //escac descobert doncs es mou fora de la direcció d'escac descobert

	piece = peca_sense_color(ss->tau.c[from]);

	if (piece == CaballBlanc) {
		if (AtacsCaball[to] & king)
			return true;
		else
			return false;
	}
	if ((piece == DamaBlanca) || (piece == AlfilBlanc) || (piece == TorreBlanca)) {
		int dir = direccio_entre_caselles(to, king_sq);
		if ((dir == 2 && ((piece == DamaBlanca) || (piece == TorreBlanca))) ||
			(dir == 1 && ((piece == DamaBlanca) || (piece == AlfilBlanc)))) {
			if (!(Entre[king_sq][to] & TTotespeces))
				return true;
		}
		return false;
	}
	if (piece == PeoBlanc) {
		if (AtacsPeo[me][to] & king)
			return true;
	}
	if (EsPromocio(m))
		return atacs_desde(PecaPromocio(m, me), to, TTotespeces ^ Bit(from)) & king;
	if (EsEnroc(m)) {
		if (COL(to) == 2)
			to = 3;
		else
			to = 5;
		if (me)
			to += 8 * 7;
		if (ROW(to) == ROW(king_sq) || COL(to) == COL(king_sq))
			if (!(Entre[king_sq][to] & (TTotespeces ^ Bit(from))))
				return true;
	}
	if (EsAlPas(m)) {
		int captura = FerCasella(ROW(from), COL(to));
		Bitboard b = (TTotespeces ^ Bit(from) ^ Bit(captura)) | Bit(to);

		if ((AtacsTorre(king_sq, b) & (Tdames(me) | Ttorres(me)))
			| (AtacsAlfil(king_sq, b) & (Tdames(me) | Talfils(me))))
			return 1;
	}
	return 0;
}

template<e_colors jo>
void generar_captures_i_coronacions_especials(tss * RESTRICT ss) {
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	Bitboard a;
	const int ell = oponent(jo);
	Bitboard contrari;
	Bitboard contrariambpeoalpas;

	if (estat_actual->nivell <= NivellRecaptures && !estat_actual->escac) {
		contrari = Bit(Desti(estat_actual->moviment_previ));
		contrariambpeoalpas = contrari;
	}
	else {
		contrari = ss->tau.peces[ell];
		contrariambpeoalpas = contrari | (estat_actual->casellapeoalpas ? Bit(estat_actual->casellapeoalpas) : 0);  //inclou peo al pas
	}
	const int increment_columna_anterior_abs = jo ? 9 : 7;
	const int increment_columna_posterior_abs = jo ? 7 : 9;
	const Bitboard filapeocorona = jo ? FILA2 : FILA7;

	//***************
	//captures de peo
	//***************

	//a la columna posterior
	//als meus peons treure la columna H
	//agafar peces del contrari i desplaçarles. Agafar les caselles on coincideixen amb els peons

	MovList * RESTRICT ml = ss->mov;

	a = (Tpeons(jo) & (~COLUMNAH)) & (ShiftPerPeons(jo, contrariambpeoalpas, increment_columna_posterior_abs));
	//posar captures amb coronacions
	int increment = jo ? -increment_columna_posterior_abs : increment_columna_posterior_abs;
	ml = peons_moviments_coronacions((a & filapeocorona), ml, increment);
	//posar captures normals
	ml = peons_moviments_potser_captures(ml, (a & (~filapeocorona)), estat_actual->casellapeoalpas, increment);

	//a la columna anterior
	a = (Tpeons(jo) & (~COLUMNAA)) & (ShiftPerPeons(jo, contrariambpeoalpas, increment_columna_anterior_abs));
	//posar captures amb coronacions
	increment = jo ? -increment_columna_anterior_abs : increment_columna_anterior_abs;
	ml = peons_moviments_coronacions((a & filapeocorona), ml, increment);
	//posar captures normals
	ml = peons_moviments_potser_captures(ml, (a & (~filapeocorona)), estat_actual->casellapeoalpas, increment);

	//posar coronacions sense captura
	//Els peons que estan per coronar
	//els que davant no tenen cap peça
	a = (Tpeons(jo) & filapeocorona) & (~ShiftPerPeons(jo, TTotespeces, 8));
	increment = Avancar(jo); //jo ? -8 : 8;
	ml = peons_moviments_coronacions(a, ml, increment);
	ss->mov = ml;
}

template<e_colors jo>
void generar_evasions_especials(tss * RESTRICT ss) {
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	Bitboard a;
	const int ell = oponent(jo);
	MovList * RESTRICT ml = ss->mov;

	if (estat_actual->destinsescac == 0) //escac doble
		goto figenev;

	//***************
	//captures de peo
	//***************
	Bitboard contrari; contrari = ss->tau.peces[ell];
	int increment_columna_anterior_abs; increment_columna_anterior_abs = jo ? 9 : 7;
	int increment_columna_posterior_abs; increment_columna_posterior_abs = jo ? 7 : 9;
	Bitboard filapeocorona; filapeocorona = jo ? FILA2 : FILA7;

	//a la columna posterior
	//als meus peons treure la columna H
	//agafar peces del contrari i desplaçarles. Agafar les caselles on coincideixen amb els peons
	a = (Tpeons(jo) & (~COLUMNAH)) & (ShiftPerPeons(jo, estat_actual->destinsescac & contrari, increment_columna_posterior_abs));
	//posar captures amb coronacions
	int increment; increment = jo ? -increment_columna_posterior_abs : increment_columna_posterior_abs;
	ml = peons_moviments_coronacions((a & filapeocorona), ml, increment);

	//a la columna anterior
	a = (Tpeons(jo) & (~COLUMNAA)) & (ShiftPerPeons(jo, estat_actual->destinsescac & contrari, increment_columna_anterior_abs));
	//posar captures amb coronacions
	increment = jo ? -increment_columna_anterior_abs : increment_columna_anterior_abs;
	ml = peons_moviments_coronacions((a & filapeocorona), ml, increment);

	//posar coronacions sense captura
	//Els peons que estan per coronar
	//els que davant no tenen cap peça			i coronant tapen l'escac
	a = (Tpeons(jo) & filapeocorona) & (~ShiftPerPeons(jo, TTotespeces, 8)) & (ShiftPerPeons(jo, estat_actual->destinsescac, 8));
	increment = Avancar(jo); //jo ? -8 : 8;
	ml = peons_moviments_coronacions(a, ml, increment);

	//parar escac amb peo al pas
	if (estat_actual->casellapeoalpas) {
		if (Bit(estat_actual->casellapeoalpas ^ 8) == estat_actual->destinsescac) {
			if (COL(estat_actual->casellapeoalpas) > 0 && (Tpeons(jo) & Bit((estat_actual->casellapeoalpas ^ 8) - 1)))
				(ml++)->moviment = FerMoviment((estat_actual->casellapeoalpas ^ 8) - 1, estat_actual->casellapeoalpas) | flag_peo_al_pas;
			if (COL(estat_actual->casellapeoalpas) < 7 && (Tpeons(jo) & Bit((estat_actual->casellapeoalpas ^ 8) + 1)))
				(ml++)->moviment = FerMoviment((estat_actual->casellapeoalpas ^ 8) + 1, estat_actual->casellapeoalpas) | flag_peo_al_pas;
		}
	}

figenev:
	ml->moviment = JugadaInvalida;
}

template<e_colors jo>
void generar_quiets_especials(tss * RESTRICT ss) {
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	const Bitboard ocupat = TTotespeces;

	MovList * RESTRICT ml = ss->mov;

	//enroc
	if (jo == blanques) {
		if ((estat_actual->enroc & PotEnrocarBlancCurt) && !(ocupat & 0x60) && !(estat_actual->atacs[negres] & 0x70))
			(ml++)->moviment = FerMoviment(4, 6) | flag_enroc;
		if ((estat_actual->enroc & PotEnrocarBlancLlarg) && !(ocupat & 0x0E) && !(estat_actual->atacs[negres] & 0x1C))
			(ml++)->moviment = FerMoviment(4, 2) | flag_enroc;
	}
	else {
		if ((estat_actual->enroc & PotEnrocarNegreCurt) && !(ocupat & 0x6000000000000000) && !(estat_actual->atacs[blanques] & 0x7000000000000000))
			(ml++)->moviment = FerMoviment(60, 62) | flag_enroc;
		if ((estat_actual->enroc & PotEnrocarNegreLlarg) && !(ocupat & 0x0E00000000000000) && !(estat_actual->atacs[blanques] & 0x1C00000000000000))
			(ml++)->moviment = FerMoviment(60, 58) | flag_enroc;
	}
	ss->mov = ml;
}

bool dins_moviments_especials(tss * RESTRICT ss, int moviment) {
	ss->mov = ss->estat_actual->moviments;
	if (ss->estat_actual->escac) {
		if (ss->estat_actual->mou == blanques)
			generar_evasions_especials<blanques>(ss);
		else
			generar_evasions_especials<negres>(ss);
	}
	else {
		if (ss->estat_actual->mou == blanques) {
			generar_captures_i_coronacions_especials<blanques>(ss);
			generar_quiets_especials<blanques>(ss);
		}
		else
		{
			generar_captures_i_coronacions_especials<negres>(ss);
			generar_quiets_especials<negres>(ss);
		}
		ss->mov->moviment = JugadaInvalida;
	}
	ss->mov = ss->estat_actual->moviments;
	MovList * RESTRICT ml = ss->estat_actual->moviments;
	for (; ml->moviment; ml++) {
		if (ml->moviment == moviment)
			return true;
	}
	return false;
}

bool jugada_ilegal(tss * RESTRICT ss, int m) {
	if (m == JugadaInvalida2)
		return true;
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	if (EsAlPas(m)) {
		int org = Origen(m);
		int mou = estat_actual->mou;
		if ((Files[ROW(org)] & Trei(mou)) &&
			(Files[ROW(org)] & Ttorresdames(oponent(mou))) &&
			(AtacsTorre(lsb(Trei(mou)), TTotespeces ^ Bit(org) ^ Bit(estat_actual->casellapeoalpas - Avancar(mou))) & Ttorresdames(oponent(mou)))) {
			return true;
		}
	}
	if (estat_actual->clavats) {
		int org = Origen(m);
		int mou = estat_actual->mou;
		if (Bit(org) & estat_actual->clavats)
			if (!(Direccio[lsb(Trei(mou))][org] & Bit(Desti(m)))) {
				return true;
			}
	}
	return false;
}

bool jugada_legal(tss * RESTRICT ss, int moviment, e_colors me) {
	int from, to, piece, capture;
	uint64_t u, occ;

	from = Origen(moviment);
	to = Desti(moviment);
	piece = ss->tau.c[from];
	capture = ss->tau.c[to];
	if (piece == 0 || from == to) return 0;
	if ((piece & 1) != me) return 0;
	if (EsEspecial(moviment))
		return dins_moviments_especials(ss, moviment);
	if (capture) {
		if ((capture & 1) == (piece & 1)) return 0;
		if (capture >= ReiBlanc) return 0;
	}
	u = Bit(to);
	occ = TTotespeces;
	uint8_t psc = peca_sense_color(piece);
	if (ss->estat_actual->escac && psc != ReiBlanc) {
		if (!(ss->estat_actual->destinsescac & u)) {  //això també verifica si és escac doble i mou peça<>rei
			return 0;
		}
	}
	if (piece >= AlfilBlanc && piece < ReiBlanc) {  //si és alfil, torre, dama
		if (Entre[from][to] & occ)
			return 0;
		int u1 = ROW(from) - ROW(to);
		int u2 = COL(from) - COL(to);
		if (psc == TorreBlanca || psc == DamaBlanca) {
			if (u1 == 0 || u2 == 0)
				return 1;
			if (psc == TorreBlanca)
				return 0;
		}
		if (abs(u1) == abs(u2))
			return 1;
		return 0;
	}
	if (psc == PeoBlanc) {
		if (to - from == Avancar(me)) {
			if (capture) return 0;
			if ((u & Fila(me, 7))) return 0; //origen a 7a i no és promoció
			return 1;
		}
		else if (to - from == Avancar(me) * 2) {
			if (capture) return 0;
			if (ss->tau.c[to - Avancar(me)]) return 0;
			if (!(u & Fila(me, 3))) return 0;
			return 1;
		}
		else if (u & AtacsPeo[me][from]) {
			if (capture == 0) return 0;
			if ((u & Fila(me, 7))) return 0; //origen a 7a i no és promoció
			return 1;
		}
		else return 0;
	}
	else if (psc == ReiBlanc) {
		if (!(AtacsRei[from] & u)) return 0;
		if (ss->estat_actual->atacs[oponent(me)] & u) return 0;

		//descobrir moviments de rei que van a casella no marcada com atacada perquè el propi rei "parava" l'atac
		if (ss->estat_actual->escac && (u & ss->estat_actual->tampocpotanarrei))
			return 0;
		return 1;
	}
	if (psc == CaballBlanc) {
		if (u & AtacsCaball[from])
			return 1;
		else
			return 0;
	}
	return 0;
}

#define PuntuacioMenjaPecaMoguda 50
bool marcar_captures_bones(tss * RESTRICT ss) {
	MovList * RESTRICT ml;
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	ml = &estat_actual->moviments[0];
	bool trobat = false;
	while (ml->moviment) {
		if (ml->moviment == JugadaInvalida2) {
			ml++;
			continue;
		}
		if (ml->moviment == estat_actual->segonmovimenthash) {
			ml->puntuacio = 10000;
			estat_actual->segonmovimenthash = JugadaInvalida; //per no tornar a intentar provar-la
			ml++;
			continue;
		}
		if (ml->moviment == estat_actual->movimenthash) {
			ml->moviment = JugadaInvalida2;
			ml++;
			continue;
		}
		int desti = Desti(ml->moviment);
		if (ss->tau.c[desti] == CasellaBuida && (!EsPromocio(ml->moviment))) { //només es produeix quan és escac
			*ss->estat_actual->seguentsegonesjugades = *ml; //això és lo mateix que posar moviment a element de seguentsegonesjugades
			estat_actual->seguentsegonesjugades++;
			ml->moviment = JugadaInvalida2;
			ml++;
			continue;
		}
		if (ss->tau.c[desti] == CasellaBuida) { //coronació
			ml->puntuacio = valorpecasee[PecaPromocio(ml->moviment, blanques)] - vpeosee;
		}
		else {
			ml->puntuacio = valorpecasee[ss->tau.c[desti]] - 78 * fila_relativa(estat_actual->mou, ROW(desti));
		}
		trobat = true;
		ml++;
	}
	return trobat;
}

void marcar_captures_bones_q(tss * RESTRICT ss) {
	MovList * RESTRICT ml;
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	const int jo = estat_actual->mou;
	const e_colors opo = (jo == blanques ? negres : blanques);
	ml = &estat_actual->moviments[0];
	for (; ml->moviment; ml++) {
		if (ml->moviment == estat_actual->movimenthash) {
			ml->moviment = JugadaInvalida2;
			ml++;
			continue;
		}
		int desti = Desti(ml->moviment);
		if (EsPromocio(ml->moviment)) {
			if (PecaPromocio(ml->moviment, blanques) == DamaBlanca) {
				ml->puntuacio = vdamasee - vpeosee;
			}
			else
				if (PecaPromocio(ml->moviment, blanques) == CaballBlanc && (Trei(oponent(jo)) & AtacsCaball[desti])) {
					ml->puntuacio = vcaballsee - vpeosee;		//fer coronacions de caball quan escac
				}
				else {
					ml->moviment = JugadaInvalida2;
				}
				continue;
		}
		ml->puntuacio = valorpecasee[ss->tau.c[desti]] - 78 * fila_relativa(jo, ROW(desti));
		if (estat_actual->moviment_previ && Desti(estat_actual->moviment_previ) == desti)
			ml->puntuacio += PuntuacioMenjaPecaMoguda;
	}
}

void puntuar_evasions_i_quiets(tss * RESTRICT ss, MovList * RESTRICT ml) {
	uint8_t pecamou;
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	//si jugada anterior era tirar endarrera, ara és menys probable que tirar endarrera sigui bo
	bool AnteriorEraTirarEndarrera = false;
	if (estat_actual->n > 1 && estat_actual->PliesDesdeNull > 2 && estat_actual->jugades50 > 2) {
		Moviment mm = (estat_actual - 1)->moviment_previ;
		if (estat_actual->mou == blanques) {
			if (ROW(Desti(mm)) < ROW(Origen(mm)))
				AnteriorEraTirarEndarrera = true;
		}
		else {
			if (ROW(Desti(mm)) > ROW(Origen(mm)))
				AnteriorEraTirarEndarrera = true;
		}
	}

	if (estat_actual->n != 1 || posathistoricobertures) {
		for (; ml->moviment; ml++) {
			if (ml->moviment == JugadaInvalida2) {
				ml->puntuacio = -(MaxPuntHistorial - 1); //per ordenar més ràpid
				continue;
			}
			if (ml->moviment == estat_actual->segonmovimenthash) { //això només passa quan és escac. No arriba mai aquí dins no sé per què
				ml->puntuacio = 10000;
				estat_actual->segonmovimenthash = JugadaInvalida; //per no tornar a intentar provar-la
				ml++;
				continue;
			}

			uint8_t origen = Origen(ml->moviment);
			uint8_t desti = Desti(ml->moviment);
			pecamou = ss->tau.c[origen];
			ml->puntuacio = PuntuacioHistorial(pecamou, desti);

			if (AnteriorEraTirarEndarrera && ml->puntuacio > -(MaxPuntHistorial - 10)) {
				if (estat_actual->mou == blanques) {
					if (ROW(origen) > ROW(desti))
						ml->puntuacio -= 10;
				}
				else {
					if (ROW(origen) < ROW(desti))
						ml->puntuacio -= 10;
				}
			}

			if (ml->puntuacio == 0) {

				if (estat_actual->mou == blanques) {
					//bonus per moviment de peo passat a 3a, 2a o coronació
					if (pecamou == PeoBlanc) {
						if (ROW(desti) > 4 && !(CasellesDavant[blanques][desti] & Tpeons(negres)))  //he provat d'usar thp->passats aquí peró és més lent
							ml->puntuacio += 5;
						// menys prioritat moure peons de prop del rei que no siguin centrals
						if (COL(origen) < 3 || COL(origen) > 4) {
							if (estat_actual->valorpeces[negres] > MaterialFinal && Distancia(lsb(Trei(blanques)), origen) == 1)	//** això de valorpeces està malament, però és més fort així
								ml->puntuacio -= 5;
						}
						else
							if (ROW(origen) == 1) //més prioritat moure 2 peons centrals des de la casella d'origen
								ml->puntuacio++;
					}
					else {
						//bonus per moviment de torre o dama a 7a o 8a
						if (pecamou == DamaBlanca || pecamou == TorreBlanca) {
							if (ROW(desti) >= 6)
								ml->puntuacio += 10;
							Bitboard b = estat_actual->atacsalfils[negres] | estat_actual->atacscaballs[negres] | estat_actual->atacspeons[negres];
							if (pecamou == DamaBlanca)
								b |= estat_actual->atacstorres[negres];
							//bonus marxa d'atac i per moure a casella no atacada
							if ((b & Bit(origen)) && !(b & Bit(desti)))
								ml->puntuacio += 5;
						}
						else
							if (pecamou == CaballBlanc && (Bit(desti) & QuadresCentrals4x4) && !(estat_actual->atacspeons[negres] & Bit(desti)))
								ml->puntuacio += 1; //caball mou cap a centre
						if (pecamou == ReiBlanc) {
							if (ROW(desti) > 0 && Tdames(negres))
								ml->puntuacio -= 5;
							else
								if (EsEnroc(ml->moviment))
									ml->puntuacio += 5;
						}
						if (estat_actual->nivell < 3 && estat_actual->atacpeoacabademoure & Bit(origen))
							ml->puntuacio += BonusMourePecaAtacadaPeo;
					}
					if (ROW(desti) > 4)
						ml->puntuacio++;
				}
				else { //mouen negres
					//bonus per moviment de peo passat a 3a, 2a o coronació
					if (pecamou == PeoNegre) {
						if (ROW(desti) < 3 && !(CasellesDavant[negres][desti] & Tpeons(blanques)))
							ml->puntuacio += 5;
						// menys prioritat moure peons de prop del rei que no siguin centrals
						if (COL(origen) < 3 || COL(origen) > 4) {
							if (estat_actual->valorpeces[blanques] > MaterialFinal && Distancia(lsb(Trei(negres)), origen) == 1)	//** això de valorpeces està malament, però és més fort així
								ml->puntuacio -= 5;
						}
						else
							if (ROW(origen) == 6) //més prioritat moure 2 peons centrals des de la casella d'origen
								ml->puntuacio++;
					}
					else {
						//bonus per moviment de torre o dama a 7a o 8a
						if (pecamou == DamaNegre || pecamou == TorreNegre) {
							if (ROW(desti) <= 1)
								ml->puntuacio += 10;
							Bitboard b = estat_actual->atacsalfils[blanques] | estat_actual->atacscaballs[blanques] | estat_actual->atacspeons[blanques];
							if (pecamou == DamaNegre)
								b |= estat_actual->atacstorres[blanques];
							//bonus marxa d'atac i per moure a casella no atacada
							if ((b & Bit(origen)) && !(b & Bit(desti)))
								ml->puntuacio += 5;

						}
						else
							if (pecamou == CaballNegre && (Bit(desti) & QuadresCentrals4x4) && !(estat_actual->atacspeons[blanques] & Bit(desti)))
								ml->puntuacio += 1; //caball mou cap a centre
						if (pecamou == ReiNegre) {
							if (ROW(desti) < 7 && Tdames(blanques))
								ml->puntuacio -= 5;
							else
								if (EsEnroc(ml->moviment))
									ml->puntuacio += 5;
						}
						if (estat_actual->nivell < 3 && estat_actual->atacpeoacabademoure & Bit(origen))
							ml->puntuacio += BonusMourePecaAtacadaPeo;
					}
					if (ROW(desti) < 3)
						ml->puntuacio++;
				}
			}
		}
		return;
	}
	for (; ml->moviment; ml++) {
		if (jugada_ilegal(ss, ml->moviment)) {
			continue;
		}
		if (ml->moviment == estat_actual->movimenthash) {
			ml->moviment = JugadaInvalida2;
			continue;
		}
		fer_moviment(ss, ml->moviment);
		ss->estat_actual++;
		calcular_atacs(ss);
		ss->estat_actual->clavats = marcar_clavats(ss, 1, ss->estat_actual->mou);
		ss->estat_actual->descoberts = marcar_clavats(ss, 0, ss->estat_actual->mou);
		ss->estat_actual->clavatsrival = marcar_clavats(ss, 1, oponent(ss->estat_actual->mou));

		ml->puntuacio = -avaluacio(ss); //negatiu pq és valoració de cara a qui mou després de la jugada
		desfer_moviment(ss, ss->estat_actual->moviment_previ);
	}
}

Moviment busca_moviment(MovList *actual, Moviment m) {
	if (!m || m == JugadaInvalida2)
		return JugadaInvalida;
	for (; actual->moviment; actual++) {
		if (actual->moviment == m) {
			actual->moviment = JugadaInvalida2;
			return m;
		}
	}
	return JugadaInvalida;
}

void insertion_sort(Estat_Analisis * RESTRICT estat_actual){
	MovList tmp, *RESTRICT p, *RESTRICT q, *RESTRICT begin, *RESTRICT end;
	begin = &estat_actual->moviments[0];
	end = estat_actual->fi - 1;

	if (end < begin)
		return; //cap quiet

	//deixar els valors - (MaxPuntHistorial - 1) al final, que son la majoria, i no ordenar - los
	while (end != begin) {
		if (end->puntuacio == -(MaxPuntHistorial - 1)) {
			end--;
			continue;
		}
		if (begin->puntuacio != -(MaxPuntHistorial - 1)) {
			begin++;
			continue;
		}
		tmp = *begin;
		*begin = *end;
		*end = tmp;
		begin++;
	}

	if (end < estat_actual->fi && end->puntuacio != -(MaxPuntHistorial - 1))
		end++;

	begin = &estat_actual->moviments[0];
	//ordenar la resta de valors
	for (p = begin + 1; p < end; ++p)
	{
		tmp = *p;
		for (q = p; q != begin && (q - 1)->puntuacio < tmp.puntuacio; --q)
			*q = *(q - 1);
		*q = tmp;
	}
}

Moviment moviment_ab(tss * RESTRICT ss){
	Estat_Analisis * RESTRICT estat_actual = ss->estat_actual;
	MovList * RESTRICT actual;
	int n;

	if (estat_actual->actual == NULL) {
	canvifase:
		estat_actual->fase++;
		switch (estat_actual->fase) {
		case etapa_bones_captures:
			estat_actual->seguentsegonesjugades = &estat_actual->segonesjugades[0];
			actual = &estat_actual->moviments[0];
			ss->mov = actual;
			if (!estat_actual->escac) {
				generar_cc(ss);
				if (!marcar_captures_bones(ss))
					goto canvifase; //** optimitzar
				goto buscar_jugada;
			}
			else {
				if (estat_actual->mou == blanques)
					generar_evasions<blanques>(ss);
				else
					generar_evasions<negres>(ss);
				if (marcar_captures_bones(ss))
					goto buscar_jugada;
			}
			estat_actual->fase++; //i continuar tot a etapa_evasions

		case etapa_evasions:
			if (!estat_actual->escac)
				goto canvifase;  //** optimitzar?
			estat_actual->seguentsegonesjugades->moviment = JugadaInvalida; //ja no se n'afegiran més
			actual = &estat_actual->segonesjugades[0];
			puntuar_evasions_i_quiets(ss, actual);
			goto buscar_jugada;

		case etapa_segonmovimenthash:
			if (estat_actual->escac)
				return JugadaInvalida; //ja ha acabat doncs ja ha provat totes les evasions
			estat_actual->actual = NULL;
			if (estat_actual->segonmovimenthash != JugadaInvalida
				&& estat_actual->segonmovimenthash != estat_actual->movimenthash
				&& jugada_legal(ss, estat_actual->segonmovimenthash, (e_colors)estat_actual->mou)) {
				return estat_actual->segonmovimenthash;
			}
			estat_actual->fase++;

		case etapa_nullmovethreat:
			estat_actual->actual = NULL;
			if (estat_actual->nullmovethreat != JugadaInvalida
				&& estat_actual->nullmovethreat != estat_actual->movimenthash
				&& estat_actual->nullmovethreat != estat_actual->segonmovimenthash
				&& ss->tau.c[Desti(estat_actual->nullmovethreat)] == CasellaBuida
				&& !EsPromocio(estat_actual->nullmovethreat)
				&& jugada_legal(ss, estat_actual->nullmovethreat, (e_colors)estat_actual->mou)) {
				return estat_actual->nullmovethreat;
			}
			estat_actual->fase++; //i continuar tot a etapa_killer1

		case etapa_killer1:
			ss->mov = &estat_actual->moviments[0];
			if (estat_actual->mou == blanques)
				generar_quiets<blanques>(ss);
			else
				generar_quiets<negres>(ss);
			ss->mov->moviment = JugadaInvalida;
			estat_actual->fi = ss->mov;
			actual = &estat_actual->moviments[0];
			n = 0;
			if (estat_actual->movimenthash)
				n++;
			if (estat_actual->segonmovimenthash && estat_actual->segonmovimenthash != estat_actual->movimenthash)
				n++;
			if (estat_actual->nullmovethreat && estat_actual->nullmovethreat != estat_actual->movimenthash && estat_actual->nullmovethreat != estat_actual->segonmovimenthash)
				n++;
			bool buscakiller0;
			buscakiller0 = false;
			if (estat_actual->killers[0] && estat_actual->killers[0] != estat_actual->movimenthash && estat_actual->killers[0] != estat_actual->segonmovimenthash && estat_actual->killers[0] != estat_actual->nullmovethreat) {
				n++;
				buscakiller0 = true;
			}
			bool hihakiller0;
			hihakiller0 = false;
			for (; n > 0 && actual->moviment; actual++) {
				if (actual->moviment == estat_actual->movimenthash || actual->moviment == estat_actual->segonmovimenthash || actual->moviment == estat_actual->nullmovethreat || actual->moviment == estat_actual->killers[0]) {
					if (buscakiller0 && actual->moviment == estat_actual->killers[0]) {
						hihakiller0 = true;
					}
					actual->moviment = JugadaInvalida2;
					n--;
				}
			}

			//buscar 1r killer
			if (hihakiller0)
				return estat_actual->killers[0];
			estat_actual->fase++; //i continuar tot a etapa_killer2


		case etapa_killer2:
			if (busca_moviment(&estat_actual->moviments[0], estat_actual->refutacio))
				return estat_actual->refutacio;
			estat_actual->fase++; //i continuar tot a etapa_killer3

		case etapa_killer3:
			if (busca_moviment(&estat_actual->moviments[0], estat_actual->killers[1]))
				return estat_actual->killers[1];
			estat_actual->fase++; //i continuar tot a etapa_quiet

		case etapa_killer4:
			if (busca_moviment(&estat_actual->moviments[0], estat_actual->refutacio3))
				return estat_actual->refutacio3;
			estat_actual->fase++; //i continuar tot a etapa_killer3

		case etapa_countermove1:
			if (busca_moviment(&estat_actual->moviments[0], estat_actual->countermoves[0]))
				return estat_actual->countermoves[0];
			estat_actual->fase++; //i continuar tot a etapa_quiet

		case etapa_countermove2:
			if (busca_moviment(&estat_actual->moviments[0], estat_actual->countermoves[1]))
				return estat_actual->countermoves[1];
			estat_actual->fase++; //i continuar tot a etapa_quiet

		case etapa_hashpeces1:
			hp_estructura * RESTRICT hp;
			hp = obtenir_hashpeces(estat_actual->hash ^ estat_actual->hashpeons);
			if (hp->clau == (estat_actual->hash ^ estat_actual->hashpeons)) {
				estat_actual->hashpeces1 = hp->move1;
				estat_actual->hashpeces2 = hp->move2;
			}
			else {
				estat_actual->hashpeces1 = JugadaInvalida;
				estat_actual->hashpeces2 = JugadaInvalida;
			}
			if (busca_moviment(&estat_actual->moviments[0], estat_actual->hashpeces1))
				return estat_actual->hashpeces1;
			estat_actual->fase++; //i continuar tot a etapa_quiet

		case etapa_hashpeces2:
			if (busca_moviment(&estat_actual->moviments[0], estat_actual->hashpeces2))
				return estat_actual->hashpeces2;
			estat_actual->fase++; //i continuar tot a etapa_quiet

		case etapa_quiet:
			estat_actual->actual = &estat_actual->moviments[0];
			puntuar_evasions_i_quiets(ss, estat_actual->actual);
			insertion_sort(estat_actual);
			goto buscar_jugada2;

		case etapa_captures_dolentes:
			estat_actual->seguentsegonesjugades->moviment = JugadaInvalida; //ja no se n'afegiran més
			actual = &estat_actual->segonesjugades[0];
			goto buscar_jugada;

		case etapa_fi:
			return JugadaInvalida;

		}
	}
	else {
	buscar_jugada2:
		if (estat_actual->fase == etapa_quiet){
		canviar_jugada:
			Moviment m = estat_actual->actual->moviment;
			if (m == JugadaInvalida)
				goto canvifase;
			estat_actual->actual++;  //ja està ordenada la llista
			if (m == JugadaInvalida2)
				goto canviar_jugada;
			return m;
		}
		actual = estat_actual->actual;
		actual++;
	}

buscar_jugada:
	MovList * RESTRICT trobat, *RESTRICT millor, ml;
	//buscar següent jugada de la fase
	for (;; actual++) {
		if (actual->moviment == JugadaInvalida)
			goto canvifase;
		if (actual->moviment == JugadaInvalida2)
			continue;
		break;
	}
	trobat = millor = actual;

	//buscar la millor jugada de la fase
	for (;;) {
		actual++;
		if (!actual->moviment)
			break;
		if (!(actual->moviment == JugadaInvalida2) &&
			actual->puntuacio > millor->puntuacio)
			millor = actual;
	}

	if (millor != trobat) {
		ml = *trobat;  //posar 1r la millor
		*trobat = *millor;
		*millor = ml;
	}
	estat_actual->actual = trobat;
	return trobat->moviment;
}

Moviment seguent_moviment(Estat_Analisis * RESTRICT estat_actual){
	MovList * RESTRICT actual = estat_actual->actual;
	while (actual->moviment == JugadaInvalida2)
		actual++;
	if (!actual->moviment)
		return JugadaInvalida;
	MovList * RESTRICT trobat, *RESTRICT millor;
	trobat = millor = actual;
	actual++;
	while (actual->moviment) {
		if (actual->moviment == JugadaInvalida2) {
			actual++;
			continue;
		}
		if (actual->puntuacio > millor->puntuacio)
			millor = actual;
		actual++;
	}
	if (millor != trobat) {
		MovList ml = *trobat;  //posar 1r la millor
		*trobat = *millor;
		*millor = ml;
	}
	estat_actual->actual = trobat + 1;
	return trobat->moviment;
}
