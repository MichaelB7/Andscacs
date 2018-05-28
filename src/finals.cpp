#include "utils.h"
#include "analisis.h"
#include "avaluacio.h"
#include "definicions.h"
#include "finals.h"
#ifdef LINUX
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//per apropar el rei al rei rival
int Apropar[8] = { 0, 0, 150, 110, 70, 40, 20, 10 };

int EnviarAlsExtrems[64] = {
	150, 130, 105, 85, 85, 105, 130, 150,
	130, 85, 60, 50, 50, 60, 85, 130,
	105, 60, 35, 25, 25, 35, 60, 105,
	85, 50, 25, 10, 10, 25, 50, 85,
	85, 50, 25, 10, 10, 25, 50, 85,
	105, 60, 35, 25, 25, 35, 60, 105,
	130, 85, 60, 50, 50, 60, 85, 130,
	150, 130, 105, 85, 85, 105, 130, 150,
};

uint8_t CasellaBlanquesIFlancDeDama(uint8_t ColorAvantatja, uint8_t CasellaPeo, uint8_t sq) {

	if (COL(CasellaPeo) >= 4)
		sq = sq ^ 7; //H1 -> A1

	if (ColorAvantatja == negres) {
		sq = (7 - ROW(sq)) * 8 + COL(sq);
	}
	return sq;
}

/* Public-domain KPK bitbase code by H.G. Muller */
#define WTM 0
#define BTM 1

char dtc[2][64][64][64]; // 512 KB
//char bitbase[2*64*64*3]; no ho uso

char steps[] = { 1, -1, 16, -16, 15, -15, 17, -17 }; // 0x88 King steps

void kpk_Init()
{ /* 3 = bK captured, 2 = can capture bK, 1 = won, 0 = undecided, -1 = broken or wK captured */
	int wk, bk, p;
	for (wk = 0; wk < 64; wk++) for (p = 0; p < 64; p++) for (bk = 0; bk < 64; bk++) {
		if (bk == wk) dtc[BTM][wk][p][bk] = 3, dtc[WTM][wk][p][bk] = -1; else
			if (wk == p) dtc[BTM][wk][p][bk] = dtc[WTM][wk][p][bk] = -1; else
				if (bk == p) dtc[BTM][wk][p][bk] = -3; else // with WTM black just captured P
					if (p >= 56) dtc[WTM][wk][p][bk] = 1; // promoted and on moviment = win
	}
}

void kpk_Pass(int stm, int first, int def, int esc)
{
	int wk, bk, p, d;
	for (wk = 0; wk < 64; wk++) for (p = 0; p < 64; p++) for (bk = 0; bk < 64; bk++) {
		int k = stm == WTM ? wk : bk;
		int O88 = (k & 070) + k; // 0x88 square number of King
		if (dtc[stm][wk][p][bk]) continue; // already decided
		dtc[stm][wk][p][bk] = def;  // assume lost if btm, undecided if wtm
		if (bk != p && stm == WTM) { // peo not captured, so can moviment
			if (p + 8 != bk && ((dtc[BTM][wk][p + 8][bk] > first) || (p < 16) &&
				((p + 16) != bk && (dtc[BTM][wk][p + 16][bk] > first))))  {
				dtc[WTM][wk][p][bk] = esc; continue;
			}
			if (p + 7 == bk && (p & 7) != 0)  { dtc[WTM][wk][p][bk] = 2; continue; }
			if (p + 9 == bk && (p & 7) != 7)  { dtc[WTM][wk][p][bk] = 2; continue; }
		}
		for (d = 0; d < 8; d++) {
			int to = O88 + steps[d];
			if (to & 0x88) continue; // off board
			to -= to >> 1 & 070;
			if (stm == WTM ? dtc[BTM][to][p][bk] > first : dtc[WTM][wk][p][to] <= first)
			{
				dtc[stm][wk][p][bk] = esc; break;
			}
		}
	}
}

//void kpk_Pack ()
//{
//  int wk, bk, p, i;
//  for(wk=0; wk<64; wk++) for(p=8; p<56; p+=2) for(bk=0; bk<64; bk++) { // calculate probe address and bit
//    int index = (bk >> 3) + 8*wk + 8*64*(p-8>>1);
//    bitbase[2*index] |= (dtc[WTM][wk][p][bk] > 0) << (bk & 7);
//    bitbase[2*index+1] |= (dtc[BTM][wk][p][bk] > 0) << (bk & 7);
//  }
//}

void kpk_Build()
{
	int i;
	kpk_Init();
	kpk_Pass(WTM, 2, 0, 2);
	kpk_Pass(BTM, 1, -2, 0); // calculate King captures and stalemates
	for (i = 0; i < 25; i++) // won't take more than 25 moves
		kpk_Pass(WTM, 0, 0, 1), kpk_Pass(BTM, 0, 1, 0);
	//kpk_Pack();
}


bool kbpk_taules_auxiliar(tss *ss)
{
	int reib, rein, peo, promocio;
	uint8_t color_peo = Talfils(blanques) ? blanques : negres;

	reib = lsb(Trei(color_peo));
	rein = lsb(Trei(oponent(color_peo)));
	//el peo més avançat, per quan hi ha doblats
	if (ss->estat_actual->mou == blanques)
		peo = msb(Tpeons(color_peo));
	else
		peo = lsb(Tpeons(color_peo));
	if (color_peo == blanques) {
		promocio = 7 * 8 + COL(peo);
	}
	else {
		promocio = COL(peo);
	}

	if (((COL(peo) == 0 || COL(peo) == 7) && colors_diferents(Talfils(color_peo), Bit(promocio)))) {
		int dist = Distancia(rein, promocio) - (ss->estat_actual->mou != color_peo);
		if ((dist < Distancia(reib, promocio) || Distancia(rein, promocio) == 1)
			&& dist <= Distancia(peo, promocio))
			return true;
	}
	return false;
}


//portar el rei a l'extrem del color de l'alfil, on es pot fer mat
int eval_KBNK(tss *ss, struct InfoEval *ie, int jo) {
	int opo = oponent(jo);
	int jcaball = lsb(Tcaballs(jo));
	int jrei = ie->rei[jo];
	int orei = ie->rei[opo];
	int c1 = 0;
	int c2 = 63;
	if (colors_diferents(Talfils(jo), 1)) {
		c1 = 7;
		c2 = 56;
	}
	int n = 0;
	if (ROW(orei) == 0 || ROW(orei) == 7 || COL(orei) == 0 || COL(orei) == 7) { //rei a extrem
		if (ROW(orei) == 0) {
			n += (7 - Distancia(orei, c1)) << 7; //bonus distancia rei oponent a extrem bo per fer-li mate
			if (Distancia(orei, c1) > 4)
				n -= 400;
			c1 ^= 7;
		}
		else {
			if (ROW(orei) == 7) {
				n += (7 - Distancia(orei, c2)) << 7; //bonus distancia rei oponent a extrem bo per fer-li mate
				if (Distancia(orei, c2) > 4)
					n -= 400;
				c1 = c2 ^ 7;
			}
			else {
				if (COL(orei) == 0) {
					if (c1 == 0) {
						n += (7 - Distancia(orei, c1)) << 5; //bonus distancia rei oponent a extrem bo per fer-li mate
						if (Distancia(orei, c1) > 4)
							n -= 400;
						c1 = 56;
					}
					else {
						n += (7 - Distancia(orei, c2)) << 5; //bonus distancia rei oponent a extrem bo per fer-li mate
						if (Distancia(orei, c2) > 4)
							n -= 400;
						c1 = 0;
					}
				}
				else {
					if (c2 == 63) {
						n += (7 - Distancia(orei, c2)) << 5; //bonus distancia rei oponent a extrem bo per fer-li mate
						if (Distancia(orei, c2) > 4)
							n -= 400;
						c1 = 7;
					}
					else {
						n += (7 - Distancia(orei, c1)) << 5; //bonus distancia rei oponent a extrem bo per fer-li mate
						if (Distancia(orei, c1) > 4)
							n -= 400;
						c1 = 63;
					}
				}
			}
		}

		if (ss->estat_actual->atacscaballs[jo] & AtacsRei[orei] & (Entre[orei][c1] | Bit(c1)))
			n += 200; //bonus caball atacant costat rei on no ha d'anar
		else
			if (ss->estat_actual->atacsalfils[jo] & AtacsRei[orei] & (Entre[orei][c1] | Bit(c1)))
				n += 200; //bonus alfil atacant costat rei on no ha d'anar
	}
	else {
		n -= 500;
		n += (7 - min(Distancia(orei, c1), Distancia(orei, c2))) << 7; //bonus distancia rei oponent a extrem bo per fer-li mate
	}
	n += (7 - Distancia(jrei, orei)) << 5; //apropar rei nostre a rei contrari
	n += (Distancia(jrei, orei) == 2) * 100; //+bonus si rei al costat
	if (ss->estat_actual->atacscaballs[jo] & AtacsRei[orei])
		n += 50; //bonus caball atacant costat rei
	if (ss->estat_actual->atacsalfils[jo] & AtacsRei[orei])
		n += 50; //bonus alfil atacant costat rei
	if (Distancia(jcaball, orei) < 3)
		n += 200; //apropar caball a rei contrari
	if (Distancia(jcaball, orei) > 4)
		n -= 500; //apropar caball a rei contrari
	return valor_victoria_coneguda_base_avaluacio + n;
}

//color=color de qui té el peo
//es retorna la puntuació de cara a qui ha de guanyar
int eval_KRPKR(tss *ss, struct InfoEval *ie, int jo) {
	int i, opo = oponent(jo);

	uint8_t reib, rein, torreb, peob, torren;

	peob = lsb(Tpeons(jo));

	//suposar que el peo està al flanc de dama per simplificar els càlculs, i que el blanc és qui té el peo
	reib = CasellaBlanquesIFlancDeDama(jo, peob, ie->rei[jo]);
	rein = CasellaBlanquesIFlancDeDama(jo, peob, ie->rei[opo]);
	torreb = CasellaBlanquesIFlancDeDama(jo, peob, lsb(Ttorres(jo)));
	torren = CasellaBlanquesIFlancDeDama(jo, peob, lsb(Ttorres(opo)));
	peob = CasellaBlanquesIFlancDeDama(jo, peob, peob);

	int c = ss->estat_actual->mou;

	if (ROW(peob) > ROW(rein) - (c == opo) && Distancia(reib, peob) < Distancia(rein, peob))
		return 100;

	//si rei controla casella coronació, peo menys de 6a fila i torre 6a fila, taules
	if (ROW(peob) < 5 && ROW(rein) > 5 && COL(peob) == COL(rein) && ROW(torren) == 5) {
		ie->escalar_avaluacio_final = 2;
		return 0;
	}

	//si el peo ja està a 6a i el rei controla la casella de coronació, es fa taules fent escac per darrera
	if (ROW(peob) >= 5 && ROW(rein) > ROW(peob) && COL(peob) == COL(rein) && ROW(torren) == 0) {
		ie->escalar_avaluacio_final = 2;
		return 0;
	}

	//si el peo està a la 6a i el rei controla la casella de coronació i el rei blanc està menys de la 6a, taules
	if (ROW(peob) == 5 && ROW(rein) > ROW(peob) && COL(peob) == COL(rein) && ROW(reib) < 5){
		ie->escalar_avaluacio_final = 2;
		return 0;
	}

	//si el rei està a la columna del peo o al costat i aquest no està massa avançat, probables taules
	if (ROW(peob) <= 3 && rein > peob) {
		if (COL(peob) == COL(rein)) {
			ie->escalar_avaluacio_final = 2;
			return 0;
		}
		if (abs(COL(peob) - COL(rein)) == 1 && Distancia(reib, rein) > 2) {
			ie->escalar_avaluacio_final = 2;
			return 0;
		}
	}

	//molts pocs casos:
	//si torreb=a8/b8/c8 i peob=a7/b7/c7, i rein a g7 o h7
	if (peob >= 48 && peob <= 51 && torreb == peob + 8 && (rein == 54 || rein == 55)) {
		//si torren en columna del peo i (li toca al negre o (li toca al blanc i no té el rei defensant el peo))
		if (COL(torren) == COL(peob) && (c != jo || !(Distancia(reib, peob) == 1))) {
			ie->escalar_avaluacio_final = 2;
			return 0;
		}
	}

	//si torre talla rei i peò a 5a i rei a la mateixa fila o superior del peo, quasi victòria
	//8/8/8/1k4R1/6K1/r5P1/8/8 b - - 0 56
	if (ROW(peob) >= 4 && ROW(reib) >= 4 && abs(COL(reib) - COL(peob)) < 2
		&& ((COL(rein) < COL(torreb) && COL(torreb) < COL(peob)) || ((COL(rein) > COL(torreb) && COL(torreb) > COL(peob)))))
		return 100;

	//si torre talla rei negre horitzontalment i rei blanc costat peò, i peo columna b o c i rei màxim 1 fila darrera peo, guanyat
	//5r2/8/8/8/8/2R5/4k1PK/8 w - - 0 1
	if (COL(peob) < 3 && COL(peob) != 0 && ROW(rein) < ROW(torreb) && ROW(torreb) <= ROW(peob) + 1 && ROW(rein) <= ROW(peob) && COL(rein) > COL(peob) + 1 && Distancia(peob, reib) < 2)
		return 100;
	
	i = 0;
	if (COL(peob) == 0)
		i -= 50; //peo lateral dona moltes més opcions de taules

	if (Distancia(rein, peob) - (c == opo) < Distancia(reib, peob) && ROW(rein) > ROW(peob))
		i -= 40;  //rei més proper dona més opcions de taules
	
	if (i)
		return i;
	else
		return puntuacioinvalida;
}

int eval_KBPKN(tss *ss, struct InfoEval *ie, int jo, int peob) {
	int opo = oponent(jo);

	uint8_t rein, alfilb;

	//suposar que el peo està al flanc de dama per simplificar els càlculs, i que el blanc és qui té el peo
	rein = CasellaBlanquesIFlancDeDama(jo, peob, ie->rei[opo]);
	alfilb = CasellaBlanquesIFlancDeDama(jo, peob, lsb(Talfils(jo)));
	peob = CasellaBlanquesIFlancDeDama(jo, peob, peob);

	//rei bloquejant i no pot ser fet fora amb escac
	if (COL(peob) == COL(rein) && rein > peob && color_casella(rein) != color_casella(alfilb))
		return 0;

	return puntuacioinvalida;
}

//color=color de qui té el peo
//es retorna la puntuació de cara a qui ha de guanyar
//peons a columna A i H opcions de taules
//8/8/k7/P7/1K6/4b3/6R1/8 b - - 0 14
int eval_KRPKB(tss *ss, struct InfoEval *ie, int jo) {
	int opo = oponent(jo);

	uint8_t reib, rein, peob, alfiln, torreb;

	peob = lsb(Tpeons(jo));

	//suposar que el peo està al flanc de dama per simplificar els càlculs, i que el blanc és qui té el peo
	reib = CasellaBlanquesIFlancDeDama(jo, peob, ie->rei[jo]);
	rein = CasellaBlanquesIFlancDeDama(jo, peob, ie->rei[opo]);
	torreb = CasellaBlanquesIFlancDeDama(jo, peob, lsb(Ttorres(jo)));
	alfiln = CasellaBlanquesIFlancDeDama(jo, peob, lsb(Talfils(opo)));
	peob = CasellaBlanquesIFlancDeDama(jo, peob, peob);
	int c = ss->estat_actual->mou;
	
	//es menja l'àlfil
	if (c == jo && (
		(AtacsPeo[jo][peob] & Bit(alfiln)) ||
		(Distancia(rein, alfiln) > 1 &&
		(
		Distancia(reib, alfiln) == 1
		|| ((COL(torreb) == COL(alfiln) || ROW(torreb) == ROW(alfiln)) && !(Entre[torreb][alfiln] & (Bit(reib) | Bit(rein) | Bit(peob))))
		)
		))
		) {
		return 150;
	}

	//torre guanya alfil
	if (c == opo && COL(torreb) == COL(alfiln) && COL(torreb) == COL(rein) && Distancia(rein, alfiln) > 2
		&& !(Entre[torreb][rein] & (Bit(reib) | Bit(peob))) && !(Entre[torreb][alfiln] & (Bit(reib) | Bit(peob))))
		return 150;

	if (c == opo && ROW(torreb) == ROW(alfiln) && ROW(torreb) == ROW(rein) && Distancia(rein, alfiln) > 2
		&& !(Entre[torreb][rein] & (Bit(reib) | Bit(peob))) && !(Entre[torreb][alfiln] & (Bit(reib) | Bit(peob))))
		return 150;

	//descartar alfil menja torre
	if (c == opo && (PrecalculAlfil[alfiln] & Bit(torreb)) && !(Entre[torreb][alfiln] & (Bit(peob) | Bit(rein))))
		return -150;

	if (c == opo && !(ss->estat_actual->atacs[jo] & Tpeons(jo))) {
		//rein menja peo
		if (AtacsRei[rein] & Bit(peob))
			return -100;
		//alfil menja peo
		if (ss->estat_actual->atacsalfils[opo] & Tpeons(jo))
			return -100;
	}

	if (c == opo && !(ss->estat_actual->atacs[jo] & Ttorres(jo))) {
		//rein menja torre
		if (AtacsRei[rein] & Bit(torreb))
			return -150;
	}

	if (c == opo && (ss->estat_actual->atacsalfils[opo] & Tpeons(jo)) && (AtacsRei[rein] & Bit(peob))) {
		//alfil menja peo defensat per torre
		if (!(AtacsRei[reib] & Bit(peob)))
			return -100;
		//alfil menja peo defensat per rei
		if (!(ss->estat_actual->atacstorres[jo] & Bit(peob)))
			return -100;
	}

	if (c == jo && ss->estat_actual->escac) {
		//alfil menja torre pq està fent escac
		if (ROW(peob) < 5 && (ss->estat_actual->atacsalfils[opo] & Ttorres(jo))
			&& (!(AtacsRei[reib] & Bit(alfiln)) || AtacsRei[rein] & Bit(alfiln))
			)
			return -150;
		//alfil menja peo pq està fent escac
		if ((ss->estat_actual->atacsalfils[opo] & Tpeons(jo)) && !(ss->estat_actual->atacs[jo] & (Tpeons(jo) | Talfils(opo))))
			return -100;
	}

	if (Distancia(rein, peob) - (c == opo) < Distancia(reib, peob) - 2
		&& ROW(peob) < ROW(rein) - (c == opo) && Distancia(rein, peob + Avancar(jo)) - (c == opo) <= 1
		&& color_casella(alfiln) == color_casella(peob))
		return -100; //més opcions de taules

	return puntuacioinvalida;
}

//jo és qui te la torre
int eval_KRKP(tss *ss, struct InfoEval *ie, int jo, bool *sortir) {
	int res = eval_KRKP_llarg(ss);
	if (res == 999)
		goto continuaKRKP;
	*sortir = false;
	if (res == 2) { //taules
		ie->escalar_avaluacio_final = 3;
		return 0;
	}
	if (res == 0)
		return -valor_victoria_coneguda_base_avaluacio; //guanya qui no té la torre
	else
		return valor_victoria_coneguda_base_avaluacio;
	continuaKRKP:
	int opo = oponent(jo);

	uint8_t reib, rein, torreb, peon;

	peon = lsb(Tpeons(opo));
	reib = ie->rei[jo];
	rein = ie->rei[opo];
	torreb = lsb(Ttorres(jo));
	//suposar que el peo és negre per simplificar els càlculs
	if (jo == negres) {
		peon = (7 - ROW(peon)) * 8 + COL(peon);
		reib = (7 - ROW(reib)) * 8 + COL(reib);
		rein = (7 - ROW(rein)) * 8 + COL(rein);
		torreb = (7 - ROW(torreb)) * 8 + COL(torreb);
	}

	if (ROW(rein) <= 2
		&& Distancia(rein, peon) == 1
		&& ROW(reib) >= ROW(peon)
		&& ((Distancia(reib, peon) - (ss->estat_actual->mou == jo) >= 2)
		|| Distancia(reib, rein) == 2)) {//Distancia(reib, rein) ==1 vol dir que el rei negre està pel mig i impedeix apropar-se
		*sortir = true;
		return 30 - Distancia(reib, peon) * 3;
	}
	else
		return puntuacioinvalida;
}

//jo és qui te la dama
int eval_KQKP(tss *ss, struct InfoEval *ie, int jo) {
	int opo = oponent(jo);

	uint8_t peo, peogeneric, caselladavantpeo;
	peo = lsb(Tpeons(opo));
	//suposar que el peo és negre per simplificar els càlculs
	if (jo == negres) {
		caselladavantpeo = peo + 8;
		peogeneric = (7 - ROW(peo)) * 8 + COL(peo);
	}
	else {
		peogeneric = peo;
		caselladavantpeo = peo - 8;
	}

	if (ROW(peogeneric) == 1 && (COL(peo) == 0 || COL(peo) == 2 || COL(peo) == 5 || COL(peo) == 7)
		&& Distancia(ie->rei[opo], peo) - (ss->estat_actual->mou == opo) <= 1  //el rei dèbil està al costat del peo o i arriba ara quan juga
		&& Distancia(ie->rei[jo], peo) - (ss->estat_actual->mou == jo) > 1 //el rei que té dama no arriba al peo
		//dama controla davant peo										rei no controla davant peo							//mou qui té la dama o el rei contrari no pot arribar a controlar la casella davant peo, per tant podrà bloquejar
		&& !((ss->estat_actual->atacsdama[jo][0] & Bit(caselladavantpeo)) && !(ss->estat_actual->atacs[opo] & Bit(caselladavantpeo)) && (ss->estat_actual->mou == jo || Distancia(ie->rei[opo], caselladavantpeo) > 1))
		//dama ja està davant del peo				//i no pot ser menjada
		&& !((lsb(Tdames(jo)) == caselladavantpeo) && !(ss->estat_actual->atacs[opo] & Bit(caselladavantpeo))))
		return 30 - Distancia(ie->rei[jo], peo) * 3;
	else
		return puntuacioinvalida;
}

int eval_KPK(tss *ss, struct InfoEval *ie, int color) {
	int win;
	if (color == blanques)
		win = dtc[ss->estat_actual->mou][ie->rei[blanques]][lsb(Tpeons(blanques))][ie->rei[negres]];
	else
		win = dtc[ss->estat_actual->mou ^ 1][(~ie->rei[negres] & 63) ^ 7][(~lsb(Tpeons(negres)) & 63) ^ 7][(~ie->rei[blanques] & 63) ^ 7];

	if (!win) {
		return 0;
	}
	return puntuacioinvalida;
}

int eval_KRKN(tss *ss, struct InfoEval *ie, int color) {
	int i = EnviarAlsExtrems[ie->rei[oponent(color)]] >> 2;  //val més tenir el rei en una cantonada
	i += Distancia(lsb(Tcaballs(oponent(color))), ie->rei[oponent(color)]) * 20; //val més tenir el caball lluny del rei
	i = i >> 1;
	return i;
}

//jo és qui té la dama
int eval_KQKR(struct InfoEval *ie, int jo) {
	int opo = oponent(jo);
	return vdamasee - vtorresee
		+ EnviarAlsExtrems[ie->rei[opo]]
		+ Apropar[Distancia(ie->rei[jo], ie->rei[opo])];
}

//Donar mate contra rei que no té material
//Portar el rei contrari a un extrem
//color = color de qui ha de donar mat
//es retorna la puntuació de cara a qui ha de donar mat
int eval_kxk(struct InfoEval *ie, int color) {
	int i = 0;

	i = Apropar[Distancia(ie->rei[negres], ie->rei[blanques])];
	i += EnviarAlsExtrems[ie->rei[oponent(color)]];  //val més tenir el rei en una cantonada
	if (color == blanques) {
		return i;
	}
	else {
		return -i;
	}
}

//detectar posició on no es pot guanyar per ofegat
int eval_KNPK(tss *ss, struct InfoEval *ie, int jo) {
	int opo = oponent(jo);

	uint8_t rein, peob;

	peob = lsb(Tpeons(jo));

	//suposar que el peo està al flanc de dama per simplificar els càlculs, i que el blanc és qui té el peo
	rein = CasellaBlanquesIFlancDeDama(jo, peob, ie->rei[opo]);
	peob = CasellaBlanquesIFlancDeDama(jo, peob, peob);

	//rei costat de (peo a A7)
	if (peob == 48 && Distancia(rein, 56) <= 1)
		return 0;

	return puntuacioinvalida;
}


//si el rei dèbil està junt a un peo que para el peo del bàndol fort, i la torre està defensada pel peo, i el rei fort no ha passat per poder atacar per darrera, opcions de taules
//jo = bàndol fort
//això son taules 8/8/8/6k1/2q3p1/4R3/5PK1/8 w - - 0 1
//en canvi això està perdut 5k2/6p1/7r/3Q1P2/5K2/8/8/8 w - - 0 1
//** perquè això funcionés del tot be en mode anàlisis, caldria que hi hagués prioritat en ordenació de moviments per posar la torre en casella defensada
void eval_KQKRP(tss *ss, struct InfoEval *ie, int jo){
	int opo = oponent(jo);

	uint8_t rein, reib, peon, peob, torren;

	//1r mirar si peo negre para el peo blanc si n'hi ha
	peon = lsb(Tpeons(opo));
	rein = CasellaBlanquesIFlancDeDama(jo, peon, ie->rei[opo]);
	reib = CasellaBlanquesIFlancDeDama(jo, peon, ie->rei[jo]);
	torren = CasellaBlanquesIFlancDeDama(jo, peon, lsb(Ttorres(opo)));

	Estat_Analisis *estat_actual = ss->estat_actual;
	if (NumPeons(jo) == 1) {
		peob = CasellaBlanquesIFlancDeDama(jo, peon, lsb(Tpeons(jo)));
		peon = CasellaBlanquesIFlancDeDama(jo, peon, peon);

		if (abs(COL(peob) - COL(peon)) != 1)
			return;
		// que el peo guanyador estigui endarrerit
		if (ROW(peob) >= ROW(peon) - 1)
			return;
		//el peò dèbil ha de estar més al centre perquè sigui segur que son taules. De l'altra forma depen
		if (COL(peob) == 2 && COL(peon) == 1)
			return;
		if (COL(peob) == 1 && COL(peon) == 0)
			return;
		//el rei dèbil ha d'estar davant del peo fort
		if (COL(rein) != COL(peob))
			return;
	}
	else
		peon = CasellaBlanquesIFlancDeDama(jo, peon, peon);

	//mirar si el rei debil està defensant el seu peo
	if (Distancia(rein, peon) > 1)
		return;

	//mirar si el rei fort no està al costat o darrera el rei dèbil
	if (ROW(reib) >= ROW(peon))
		return;

	//que la torre estigui defensada pel seu peo
	if (!(peon - torren == 7 || peon - torren == 9))
		return;

	ie->escalar_avaluacio_final = 3;
}


//torna una puntuació sense separar mig joc i final
int mirar_si_final_especialitzat(tss *ss, struct InfoEval *ie, bool *sortir){
	Estat_Analisis *estat_actual = ss->estat_actual;
	*sortir = true;
	int i = 0;

	//taules per poc material
	if (estat_actual->numpeces[PeoBlanc] == 0 && estat_actual->numpeces[PeoNegre] == 0) {
		if ((ie->vpb < vtorre && ie->vpn < vtorre)
			|| (ie->vpb == vtorre && ie->vpn == vtorre)
			) {
			*sortir = false;
			ie->escalar_avaluacio_final = 2;
			return 0;
		}
		//dos menors contra 1 menor
		if (estat_actual->numpeces[AlfilBlanc] + estat_actual->numpeces[CaballBlanc] + estat_actual->numpeces[AlfilNegre] + estat_actual->numpeces[CaballNegre] == 3 && (abs((int)((estat_actual->numpeces[AlfilBlanc] + estat_actual->numpeces[CaballBlanc]) - (estat_actual->numpeces[AlfilNegre] + estat_actual->numpeces[CaballNegre]))) == 1)) {
			*sortir = false;
			if (!((estat_actual->numpeces[AlfilBlanc] == 2 && estat_actual->numpeces[CaballNegre] == 1) || (estat_actual->numpeces[AlfilNegre] == 2 && estat_actual->numpeces[CaballBlanc] == 1))) //KBBKN guanyat molts cops
				ie->escalar_avaluacio_final = 3;
			return 0;
		}

		//KBNK
		if (ie->vpb == valfil + vcaball && ie->vpn == 0) {
			return eval_KBNK(ss, ie, blanques);
		}
		if (ie->vpn == valfil + vcaball && ie->vpb == 0) {
			return -eval_KBNK(ss, ie, negres);
		}

		//KRKB, s'intenta portar el rei als extrems
		if (ie->vpb == vtorre && ie->vpn == valfil) {
			i = 0;
			//bonificació específica per rei enfrontat a rei quan està atrapat en un extrem
			//8/8/6B1/K1k5/1r6/8/8/8 w - - 0 131
			//2k5/R7/3K4/8/b7/8/8/8 b - - 0 189
			if (ROW(ie->rei[negres]) == 0 && ROW(ie->rei[blanques]) == 2 && COL(ie->rei[blanques]) == COL(ie->rei[negres]))
				i += 20;
			if (ROW(ie->rei[negres]) == 7 && ROW(ie->rei[blanques]) == 5 && COL(ie->rei[blanques]) == COL(ie->rei[negres]))
				i += 20;
			if (COL(ie->rei[negres]) == 0 && COL(ie->rei[blanques]) == 2 && ROW(ie->rei[blanques]) == ROW(ie->rei[negres]))
				i += 20;
			if (COL(ie->rei[negres]) == 7 && COL(ie->rei[blanques]) == 5 && ROW(ie->rei[blanques]) == ROW(ie->rei[negres]))
				i += 20;
			if (i) {
				if (color_casella(ie->rei[blanques]) != color_casella(lsb(Talfils(negres))))  //l'àlfil no pot fer fora el rei amb escac
					i += 20;
				//millor per la defensa portar el rei al costat del color contrari a l'àlfil
				if (color_casella(lsb(Talfils(negres))) == color_casella(0))
					i -= 7 - (min(Distancia(ie->rei[negres], 7), Distancia(ie->rei[negres], 56)));
				else
					i -= 7 - (min(Distancia(ie->rei[negres], 0), Distancia(ie->rei[negres], 63)));
			}
			i += eval_kxk(ie, blanques) >> 3;
			return i;
		}
		if (ie->vpn == vtorre && ie->vpb == valfil) {
			i = 0;
			if (ROW(ie->rei[blanques]) == 0 && ROW(ie->rei[negres]) == 2 && COL(ie->rei[negres]) == COL(ie->rei[blanques]))
				i -= 20;
			if (ROW(ie->rei[blanques]) == 7 && ROW(ie->rei[negres]) == 5 && COL(ie->rei[negres]) == COL(ie->rei[blanques]))
				i -= 20;
			if (COL(ie->rei[blanques]) == 0 && COL(ie->rei[negres]) == 2 && ROW(ie->rei[negres]) == ROW(ie->rei[blanques]))
				i -= 20;
			if (COL(ie->rei[blanques]) == 7 && COL(ie->rei[negres]) == 5 && ROW(ie->rei[negres]) == ROW(ie->rei[blanques]))
				i -= 20;
			if (i) {
				if (color_casella(ie->rei[negres]) != color_casella(lsb(Talfils(blanques))))  //l'àlfil no pot fer fora el rei amb escac
					i -= 20;
				//millor per la defensa portar el rei al costat del color contrari a l'àlfil
				if (color_casella(lsb(Talfils(blanques))) == color_casella(0))
					i += 7 - (min(Distancia(ie->rei[blanques], 7), Distancia(ie->rei[blanques], 56)));
				else
					i += 7 - (min(Distancia(ie->rei[blanques], 0), Distancia(ie->rei[blanques], 63)));
			}
			i += eval_kxk(ie, negres) >> 3;
			return i;
		}

		//KRKN, s'intenta portar el rei als extrems
		if (ie->vpb == vtorre && ie->vpn == vcaball) {
			return eval_KRKN(ss, ie, blanques);
		}
		if (ie->vpn == vtorre && ie->vpb == vcaball) {
			return -eval_KRKN(ss, ie, negres);
		}

		//KQKR
		if (ie->vpb == vdama && ie->vpn == vtorre) {
			return eval_KQKR(ie, blanques);
		}
		if (ie->vpn == vdama && ie->vpb == vtorre) {
			return -eval_KQKR(ie, negres);
		}

		//KRRKRB, probables taules
		if (ie->vpb ==  vtorre + vtorre && ie->vpn == valfil + vtorre) {
			ie->escalar_avaluacio_final = 2;
			*sortir = false;
			return 0;
		}
		if (ie->vpn == vtorre + vtorre && ie->vpb == valfil + vtorre) {
			ie->escalar_avaluacio_final = 2;
			*sortir = false;
			return 0;
		}

		//KRRKRN, probables taules
		if (ie->vpb == vtorre + vtorre && ie->vpn == vcaball + vtorre) {
			ie->escalar_avaluacio_final = 1;
			*sortir = false;
			return 0;
		}
		if (ie->vpn == vtorre + vtorre && ie->vpb == vcaball + vtorre) {
			ie->escalar_avaluacio_final = 1;
			*sortir = false;
			return 0;
		}

		if (Tdames(blanques) && ie->vpn <= vtorre) {
			return valor_victoria_coneguda_base_avaluacio + min(3000, ie->vpb) + eval_kxk(ie, blanques);
		}
		if (Tdames(negres) && ie->vpb <= vtorre) {
			return -((valor_victoria_coneguda_base_avaluacio + min(3000, ie->vpn)) - eval_kxk(ie, negres));
		}
	}

	if (estat_actual->numpeces[PeoBlanc] + estat_actual->numpeces[PeoNegre] == 1) {
		//KRPKR
		if (ie->vpb == vtorre + vpeo && ie->vpn == vtorre) {
			//blanc té el peo
			i = eval_KRPKR(ss, ie, blanques);
			*sortir = false;
			if (i != puntuacioinvalida) {
				return i;
			}
			return 0;
		}
		if (ie->vpn == vtorre + vpeo && ie->vpb == vtorre) {
			//negre té el peo
			i = -eval_KRPKR(ss, ie, negres);
			*sortir = false;
			if (i != -puntuacioinvalida) {
				return i;
			}
			return 0;
		}

		//KRKP
		if (ie->vpb == vtorre && ie->vpn == vpeo) {
			//blanc té la torre
			i = eval_KRKP(ss, ie, blanques, sortir);
			if (i != puntuacioinvalida) {
				return i;
			}
			*sortir = false;
			return 0;
		}
		if (ie->vpn == vtorre && ie->vpb == vpeo) {
			//negre té la torre
			i = -eval_KRKP(ss, ie, negres, sortir);
			if (i != -puntuacioinvalida) {
				return i;
			}
			*sortir = false;
			return 0;
		}

		//KQKP
		if (ie->vpb == vdama && ie->vpn == vpeo) {
			//blanc té la dama
			i = eval_KQKP(ss, ie, blanques);
			if (i != puntuacioinvalida) {
				return i;
			}
			*sortir = false;
			return 0;
		}
		if (ie->vpn == vdama && ie->vpb == vpeo) {
			//negre té el dama
			i = -eval_KQKP(ss, ie, negres);
			if (i != -puntuacioinvalida) {
				return i;
			}
			*sortir = false;
			return 0;
		}

		//KBPKN
		if (ie->vpb == valfil + vpeo && ie->vpn == vcaball) {
			//blanc té el peo
			i = eval_KBPKN(ss, ie, blanques, lsb(Tpeons(blanques)));
			*sortir = false;
			if (i != puntuacioinvalida) {
				ie->escalar_avaluacio_final = 5;
				return i;
			}
			return 0;
		}
		if (ie->vpn == valfil + vpeo && ie->vpb == vcaball) {
			//negre té el peo
			i = -eval_KBPKN(ss, ie, negres, lsb(Tpeons(negres)));
			*sortir = false;
			if (i != -puntuacioinvalida) {
				ie->escalar_avaluacio_final = 5;
				return i;
			}
			return 0;
		}

		//KPK
		if (ie->vpb == vpeo && ie->vpn == 0) {
			//blanc té el peo
			i = eval_KPK(ss, ie, blanques);
			if (i != puntuacioinvalida/* && color_que_juga_pc==negres*/) {
				return i; //son taules
			}
			*sortir = false;
			return valor_victoria_coneguda_base_avaluacio;
		}
		if (ie->vpn == vpeo && ie->vpb == 0) {
			//negre té el peo
			i = -eval_KPK(ss, ie, negres);
			if (i != -puntuacioinvalida/* && color_que_juga_pc==blanques*/) {
				return i;  //son taules
			}
			*sortir = false;
			return -valor_victoria_coneguda_base_avaluacio;
		}

		//KRPKB peons columnes A i H opcions de taules
		if (ie->vpb == vtorre + vpeo && ie->vpn == valfil) {
			//blanc té el peo
			i = eval_KRPKB(ss, ie, blanques);
			*sortir = false;
			if (i != puntuacioinvalida) {
				return i;
			}
			return 0;
		}
		if (ie->vpn == vtorre + vpeo && ie->vpb == valfil) {
			//negre té el peo
			i = -eval_KRPKB(ss, ie, negres);
			*sortir = false;
			if (i != -puntuacioinvalida) {
				return i;
			}
			return 0;
		}

		//KNPK detectar posició on no es pot guanyar per ofegat
		if (ie->vpb == vcaball + vpeo && ie->vpn == 0) {
			//blanc té el peo
			i = eval_KNPK(ss, ie, blanques);
			*sortir = false;
			if (i != puntuacioinvalida) {
				ie->escalar_avaluacio_final = 6;
				return i;
			}
			return 0;
		}
		if (ie->vpn == vcaball + vpeo && ie->vpb == 0) {
			//negre té el peo
			i = -eval_KNPK(ss, ie, negres);
			*sortir = false;
			if (i != -puntuacioinvalida) {
				ie->escalar_avaluacio_final = 6;
				return i;
			}
			return 0;
		}
	}

	//KBPK amb possibles varis peons doblats en una columna extrema
	if (ie->vpb_np == valfil
		&& ie->vpn_np == 0 && estat_actual->valorpeces[blanques] > estat_actual->valorpeces[negres]
		&& (((Tpeons(blanques) & COLUMNAA) == Tpeons(blanques) && color_casella(lsb(Talfils(blanques))) == negres)
		|| ((Tpeons(blanques) & COLUMNAH) == Tpeons(blanques) && color_casella(lsb(Talfils(blanques))) == blanques))
			) {
			*sortir = false;
			i = kbpk_taules(ss);
			if (i == 1) {
				ie->escalar_avaluacio_final = 5;
				return 0;
			}
			if (i == valor_victoria_coneguda_base_avaluacio) {
				if (Tpeons(negres))
					return 100;
				return i;
			}
			return 0;
	}
	
	if (ie->vpn_np == valfil
		&& ie->vpb_np == 0 && estat_actual->valorpeces[negres] > estat_actual->valorpeces[blanques]
		&& (((Tpeons(negres) & COLUMNAA) == Tpeons(negres) && color_casella(lsb(Talfils(negres))) == blanques)
		|| ((Tpeons(negres) & COLUMNAH) == Tpeons(negres) && color_casella(lsb(Talfils(negres))) == negres))
		) {
		*sortir = false;
		i = kbpk_taules(ss);
		if (i == 1) {
			ie->escalar_avaluacio_final = 5;
			return 0;
		}
		if (i == valor_victoria_coneguda_base_avaluacio) {
			if (Tpeons(blanques))
				return -100;
			return -i;
		}
		return 0;
	}

	//finals amb un peó de més
	if (ie->vpb_np == 0 && ie->vpn_np == 0) {
		if (abs((int)(estat_actual->numpeces[PeoBlanc] - estat_actual->numpeces[PeoNegre])) == 1) {
			//aquí no hi pot arribar si kpk
			*sortir = false;
			if (estat_actual->numpeces[PeoBlanc] > estat_actual->numpeces[PeoNegre])
				return 30;
			else
				return -30;
		}
	}

	//KPnK però peons de a/h i bloquejats per rei, taules
	if (ie->vpb == 0 && ie->vpn_np == 0 && Tpeons(negres)) {
		if (Tpeons(negres) == (Tpeons(negres) & COLUMNAA) || Tpeons(negres) == (Tpeons(negres) & COLUMNAH)) { //peons a A o a H
			if (AvancPeonsPassats[negres][lsb(Tpeons(negres))] & Trei(blanques)) { //rei davant a màxim 1 columna dels peons
				ie->escalar_avaluacio_final = 5;
				return i;
			}
		}
	}
	if (ie->vpn == 0 && ie->vpb_np == 0 && Tpeons(blanques)) {
		if (Tpeons(blanques) == (Tpeons(blanques) & COLUMNAA) || Tpeons(blanques) == (Tpeons(blanques) & COLUMNAH)) { //peons a A o a H
			if (AvancPeonsPassats[blanques][msb(Tpeons(blanques))] & Trei(negres)) { //rei davant a màxim 1 columna dels peons
				ie->escalar_avaluacio_final = 5;
				return i;
			}
		}
	}
		
	if (estat_actual->numpeces[PeoBlanc] == 2 || estat_actual->numpeces[PeoNegre] == 2) {
		//KBPPKN on peo doblat
		if (ie->vpb == valfil + vpeo + vpeo && ie->vpn == vcaball) {
			*sortir = false;
			if (COL(lsb(Tpeons(blanques))) == COL(msb(Tpeons(blanques)))) {
				//blanc té el peo
				i = eval_KBPKN(ss, ie, blanques, msb(Tpeons(blanques)));
				if (i != puntuacioinvalida) {
					ie->escalar_avaluacio_final = 5;
					return i;
				}
			}
			return 0;
		}
		if (ie->vpn == valfil + vpeo + vpeo && ie->vpb == vcaball) {
			*sortir = false;
			if (COL(lsb(Tpeons(negres))) == COL(msb(Tpeons(negres)))) {
				//negre té el peo
				i = -eval_KBPKN(ss, ie, negres, lsb(Tpeons(negres)));
				if (i != -puntuacioinvalida) {
					ie->escalar_avaluacio_final = 5;
					return i;
				}
			}
			return 0;
		}
	}

	//fortalesa dama contra torre
	if (ie->vpb_np == vdama
		&& ie->vpn == vtorre + vpeo
		&& estat_actual->numpeces[PeoBlanc] <= 1) {
		eval_KQKRP(ss, ie, blanques);
		*sortir = false;
		//si de cas haurà modificat escalar_avaluacio_final
		return 0;
	}
	if (ie->vpn_np == vdama
		&& ie->vpb == vtorre + vpeo
		&& estat_actual->numpeces[PeoNegre] <= 1) {
		eval_KQKRP(ss, ie, negres);
		*sortir = false;
		//si de cas haurà modificat escalar_avaluacio_final
		return 0;
	}

	//1 o 2 peons
	if (estat_actual->numpeces[TorreBlanca] == 0 && estat_actual->numpeces[DamaBlanca] == 0 && estat_actual->numpeces[TorreNegre] == 0 && estat_actual->numpeces[DamaNegre] == 0) {
		if (estat_actual->numpeces[PeoBlanc] + estat_actual->numpeces[PeoNegre] < 3) {
			//2 menors contra un alfil i peons
			if (estat_actual->numpeces[PeoBlanc] == 0 && estat_actual->numpeces[AlfilBlanc] + estat_actual->numpeces[CaballBlanc] == 2 && estat_actual->numpeces[AlfilNegre]/* + estat_actual->numpeces[CaballNegre] == 1*/) {
				ie->escalar_avaluacio_final = 4;
				*sortir = false;
				return 0;
			}
			if (estat_actual->numpeces[PeoNegre] == 0 && estat_actual->numpeces[AlfilNegre] + estat_actual->numpeces[CaballNegre] == 2 && estat_actual->numpeces[AlfilBlanc]/* + estat_actual->numpeces[CaballBlanc] == 1*/) {
				ie->escalar_avaluacio_final = 4;
				*sortir = false;
				return 0;
			}
		}
		//1 peo i menor vs 2 peons KNPKPP 
		if (estat_actual->numpeces[PeoBlanc] == 1 && estat_actual->numpeces[PeoNegre] == 2 && estat_actual->numpeces[AlfilBlanc] + estat_actual->numpeces[CaballBlanc] == 1 && estat_actual->numpeces[AlfilNegre] + estat_actual->numpeces[CaballNegre] == 0) {
			*sortir = false;
			return 150; //final normalment guanyat
		}
		if (estat_actual->numpeces[PeoBlanc] == 2 && estat_actual->numpeces[PeoNegre] == 1 && estat_actual->numpeces[AlfilBlanc] + estat_actual->numpeces[CaballBlanc] == 0 && estat_actual->numpeces[AlfilNegre] + estat_actual->numpeces[CaballNegre] == 1) {
			*sortir = false;
			return -150; //final normalment guanyat
		}
	}


	//KRR(B/N) KRR(P), probables taules
	if ((ie->vpb == vtorre + vtorre + valfil || ie->vpb == vtorre + vtorre + vcaball)
		&& (ie->vpn == vtorre + vtorre + vpeo || ie->vpn == vtorre + vtorre)) {
		ie->escalar_avaluacio_final = 2;
		*sortir = false;
		return 0;
	}
	if ((ie->vpn == vtorre + vtorre + valfil || ie->vpn == vtorre + vtorre + vcaball)
		&& (ie->vpb == vtorre + vtorre + vpeo || ie->vpb == vtorre + vtorre)) {
		ie->escalar_avaluacio_final = 2;
		*sortir = false;
		return 0;
	}


	if (i == 0) {
		//Mats bàsics contra rei sol, acorralar-lo
		//Està desprès de la comprovació dels altres finals, per donar preferència a mats especials contra rei, per exemple KBNK
		if (ie->vpb == 0 && ie->vpn >= vtorre) {   // && estat_actual->valorpeces[negres]==valorpeca[TorreBlanca]
			return -((valor_victoria_coneguda_base_avaluacio + min(3000, ie->vpn)) - eval_kxk(ie, negres)); // es suma estat_actual->valorpeces[negres] perquè seleccioni entrar en un final amb el màxim de peces
		}
		if (ie->vpn == 0 && ie->vpb >= vtorre) {
			return valor_victoria_coneguda_base_avaluacio + min(3000, ie->vpb) + eval_kxk(ie, blanques);  // es suma estat_actual->valorpeces[blanques] perquè seleccioni entrar en un final amb el màxim de peces
		}
	}

	*sortir = false;
	return i;
}