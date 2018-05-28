#include "utils.h"
#include "analisis.h"
#include "avaluacio.h"
#include "definicions.h"
#include "finals.h"
#include "magics.h"

#ifdef LINUX
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

int kbpk_taules(tss *ss)
{
	int wk, bk, b, p, c, x;
	int color_peo = Talfils(blanques) ? blanques : negres;

	wk = lsb(Trei(color_peo));
	bk = lsb(Trei(oponent(color_peo)));
	b = lsb(Talfils(color_peo));
	p = lsb(Tpeons(color_peo));

	c = ss->estat_actual->mou;
	if (color_peo == negres) {
		//passar a com si fossin blanques
		wk = (7 - ROW(wk)) * 8 + COL(wk);
		bk = (7 - ROW(bk)) * 8 + COL(bk);
		b = (7 - ROW(b)) * 8 + COL(b);
		p = (7 - ROW(p)) * 8 + COL(p);
		c = oponent(c);
	}

	if (COL(p) == 0) {
		wk = wk ^ 7; //a1 -> h1
		bk = bk ^ 7;
		b = b ^ 7;
		p = p ^ 7;
	}

	Bitboard atalfil = AtacsAlfil(b, (Bitboard)(Bit(wk) | Bit(bk)));
	Bitboard atrei = AtacsRei[wk];
	Bitboard atblanques = atalfil | atrei | AtacsPeo[blanques][p];

	int unknown = 999;
	int own = unknown;

	if (ROW(bk) < ROW(p) && c == blanques && (COL(b) != 7 || ROW(b) < ROW(p))) //pawn scapes
		own = 2; //237166

	//pawn blocked
	//exceptions: 8/8/8/5B2/7k/2K5/7P/8 w - - 0 1
	if (own == unknown && COL(bk) == 7 && ROW(bk) > ROW(p) && ROW(bk) > 5)
		own = 0; //37299

	//pawn can be captured
	if (own == unknown && Distancia(bk, p) == 1) {
		if (!(atblanques & Bit(p)))
			if (c == negres || (b == FerCasella(7, ROW(p) + 1) &&
				(Distancia(wk, p) > 2 || wk == FerCasella(5, ROW(p) - 1)) //second verification implies black king prevents white king to defend the pawn
				)) //black's turn or pawn blocked by bishop and king does not arrive to defend it
				own = 0; //42194
	}

	//pawn can be captured because is blocked and undefendable
	if (own == unknown && ((Distancia(bk, p) == 2 && c == negres) || (Distancia(bk, p) == 1 && c == blanques))
		&& b == FerCasella(7, ROW(p) + 1) && (Distancia(wk, p) > 2 || (ROW(wk) <= ROW(bk) && COL(wk) < COL(bk))) && ROW(bk) <= ROW(p))
		own = 0; //561

	if (own == unknown && Distancia(bk, cH8) == 1)
		own = 0; //21244

	//stalemate
	if (own == unknown && c == negres && (
		((COL(bk) == 0 || COL(bk) == 7) && Distancia(bk, b) == 1 && Distancia(bk, wk) == 2 && ROW(b) == ROW(bk) && ROW(wk) == ROW(bk))
		||
		((ROW(bk) == 0 || ROW(bk) == 7) && Distancia(bk, b) == 1 && Distancia(bk, wk) == 2 && COL(b) == COL(bk) && COL(wk) == COL(bk))
		)
		)
		own = 0; //81

	//stalemate
	if (own == unknown && c == negres && (
		(bk == cA1 && (atblanques & A2) && (atblanques & B1))
		||
		(bk == cA8 && (atblanques & A7) && (atblanques & B8))
		||
		(bk == cH1 && (atblanques & H2) && (atblanques & G1))
		||
		(bk == cH8 && (atblanques & H7) && (atblanques & G8))
		||
		((bk == cF8 && (atblanques & F7) && (atblanques & G8)) && (wk == cD8 || wk == cD7))
		||
		(bk == cF8 && b == cF7 && p == cH6 && wk == cE6)
		)
		)
		own = 0; //223

	//stalemate
	if (own == unknown && c == negres && wk == cF6 && p == cH7 && bk == cF8 && (atalfil & E8))
		own = 0; //6

	if (own == unknown && ROW(bk) < ROW(p) - 1 && c == negres && (COL(b) != 7 || ROW(b) < ROW(p))) //pawn scapes
		own = 2; //197207

	if (own == unknown && ROW(p) == 1 && ROW(bk) <= 1 && (COL(b) != 7 || ROW(b) < ROW(p))) {//pawn scapes advancing 2 squares
		if (c == blanques || (ROW(bk) == 0 && (Distancia(wk, p) == 1 || Distancia(bk, p) > 1)))  //white's turn or pawn defended and black cannot block it or black cannot take pawn
			own = 2; //18758
	}

	if (own == unknown && ROW(bk) + 2 <= ROW(p) && c == blanques) //pawn scapes
		own = 2; //5050

	//black takes bishop and stalemates white
	if (own == unknown && wk == cH8 && p == cH7 && c == negres && b == cF7 && Distancia(bk, b) == 1)
		own = 0; //6

	if (own == unknown && COL(bk) + (c == negres) < ROW(p) && COL(b) != 7)//pawn scapes
		own = 2; //196445

	//white king prevents black king to reach the pawn or h file
	if (own == unknown && c == blanques && COL(wk) > COL(bk) && abs(ROW(wk) - ROW(bk)) < 2 && COL(wk) < 7)
		own = 2; //11227

	//white king prevents black king to reach the pawn or h file
	if (own == unknown && c == blanques && COL(wk) > COL(bk) && ROW(bk) <= ROW(wk) && COL(wk) < 7)
		own = 2; //9710

	//white king prevents black king to reach the pawn 
	if (own == unknown && (ROW(bk) < ROW(p) || (ROW(bk) == ROW(p) && c == blanques)) &&
		(ROW(wk) > ROW(bk) || (ROW(wk) == ROW(bk) && (COL(wk) < 7 || COL(bk) < 4)))
		&& (COL(wk) >= COL(bk) || bk == cB1 || (bk == cC1 && c == blanques)) &&
		(
		Distancia(bk, p) - (c == negres) > Distancia(wk, p) ||
		(Distancia(bk, p) - (c == negres) == Distancia(wk, p) && (COL(wk) == COL(bk) || (COL(wk) >= COL(bk) && Distancia(wk, bk) < 5))) ||
		(Distancia(bk, p) - (c == negres) + 1 == Distancia(wk, p) && COL(wk) >= COL(bk) && (ROW(p) == 1 || ROW(p) - ROW(bk) > 1)) ||
		(bk == cB1 || (bk == cC1 && c == blanques))
		)
		)
		own = 2; //11342

	//black king far of pawn
	if (own == unknown && (bk == cA1 ||
		Distancia(bk, cG7) > 8 - ROW(p) - (c == blanques)
		))
		own = 2; //14983

	//black king can't stop pawn even near of it
	if (own == unknown && c == negres && p == cH6 && (
		(atblanques & F7) &&
		((bk == cF6 && (atblanques & G6)) ||
		(bk == cF8 && (atblanques & G8))
		)
		))
		own = 2; //412

	//black king stops pawn
	if (own == unknown && COL(bk) > COL(wk) && ROW(bk) - 1 > ROW(p) && COL(bk) >/*=*/ROW(p) && c == negres && Distancia(wk, cF6) > 2)
		own = 0; //52286

	//pawn scapes
	if (own == unknown && Distancia(bk, cH8) - (c == negres) > Distancia(p, cH8) && (COL(b) != 7 || ROW(b) < ROW(p)))
		own = 2; //4013

	//pawn no reachable because bishop prevents pass
	if (own == unknown && b == cH5 && (p == cH4 || (p < cH4 && c == blanques)) && Distancia(bk, cH8) - (c == negres)> 4 &&
		Distancia(bk, cH4) >= Distancia(bk, cH8) &&
		Distancia(bk, cH4) <= Distancia(bk, cF8)
		)
		own = 2; //94  <<---------------

	//per cada posició de rei blanc mirar les que guanyen sempre; idem per combinacions de wk, bk

	//black king stops the pawn thanks to bishop blocking it
	if (own == unknown && Distancia(bk, cH8) - (c == negres) </*=*/ Distancia(p, cH8) - (p == cH2) && COL(b) == 7 && ROW(b) > ROW(p) &&
		COL(wk) < COL(bk) &&
		!(Distancia(wk, cF7) - (c == blanques) <= Distancia(bk, cH6)) &&
		!(wk == cG8 || wk == cH8) &&
		!(b == cH7 && (p == cH6 || (p == cH5 && c == blanques)))
		)
		own = 0; //3594


	if (own == unknown && bk == cF6 && p == cH6) {
		//pawn no reachable because bishop prevents pass
		if (b == cE8 || b == cH5)
			own = 2; //98
		else {
			if (c == negres && Distancia(p, wk) > 3) //black takes or captures the pawn
				own = 0; //1137
			else
				if (c == blanques &&
					(
					(atalfil & H5)
					||
					(atalfil & E8)
					))  //bishop can block king to reach the pawn
					own = 2; //480  
		}
	}

	if (own == unknown && bk == cE5 && p == cH5) {
		//pawn no reachable because bishop prevents pass
		if (c == blanques && (
			(atalfil & H5)
			||
			(atalfil & E8)
			))  //bishop can block king to reach the pawn
			own = 2;  //446
	}

	// black king does not have time to attack the undefended h6 pawn
	//5k2/7B/7P/8/8/8/3K4/8 w - - 0 1
	if (own == unknown && b == cH7 && p == cH6 && COL(bk) < 6 && ROW(bk) > 5 && ROW(wk) < ROW(bk) && Distancia(wk, p) - (c == blanques) < 4)
		own = 2; //174

	//black king stops the pawn
	if (own == unknown && (p == cH2 || p == cH3) && (b == cH3 || b == cH5 || b == cH7) &&
		(COL(bk) > 2 || (COL(bk) == 2 && c == negres && ROW(wk) <= ROW(bk))) && COL(wk) < 2 && (ROW(bk) > 0 || (ROW(bk) == 0 && c == negres && COL(wk) == 0)) &&
		(c == negres || (c == blanques &&
		(COL(bk) - COL(wk) > 3 || ROW(wk) <= ROW(bk))
		)
		))
		own = 0; //1244

	//black king stops pawn
	if (own == unknown && p == cH7 && c == negres && Distancia(bk, cG7) == 1 && Distancia(wk, cG7) > 1)
		own = 0; //5629

	//black king stops pawn
	if (own == unknown && p == cH6 && c == negres && Distancia(bk, cG7) <= 2 && Distancia(wk, cG7) > 1 && Distancia(wk, cE7) > 1 && Distancia(wk, cF4) > 2 &&
		!(
		(atalfil & H5)
		||
		(atalfil & E8)
		||
		(atalfil & H7)
		)
		&& !((bk == cF8 || bk == cE8) &&
		(atalfil & G8)
		)
		)
		own = 0; //2611

	if (own == unknown && p == cH7 && Distancia(wk, cG7) == 1)
		own = 2; //382

	//black king takes the bishop and kpk endgame is draw
	if (own == unknown && c == negres && Distancia(bk, b) == 1 && !(atblanques & Bit(b))) {
		/*pos.remove_piece(blanques, BISHOP, b);  //some exceptions when bishop checking and defending indirectly the pawn
		pos.move_piece(negres, KING, bk, b);
		pos.sideToMove = ~pos.sideToMove;
		pos.set_state(pos.st);
		int vv = Tablebases::probe_wdl(pos, &found);
		if (vv == 0)
		own = 0;
		pos.put_piece(negres, KING, bk);
		pos.put_piece(blanques, BISHOP, b);
		pos.sideToMove = ~pos.sideToMove;
		pos.set_state(pos.st);*/
		return false; //avaluar
	} //7977

	//bishop stops black king to reach the pawn
	if (own == unknown && p == cH6 && (wk == cG7 ||
		(b == cH5 || b == cG6) && (bk == cE6 || (bk != cF7 && Distancia(bk, cF8) - (c == negres) > 1))))
		own = 2; //410

	//wk helps pawn advance from front of it
	if (own == unknown && ROW(wk) > ROW(p) && COL(wk) == 7 && Distancia(wk, p) <= Distancia(bk, p) && ROW(bk) < ROW(wk) + (c == blanques))
		own = 2; //9687

	if (own == unknown &&  p == cH6) {
		if (b == cH7) {
			if (bk == cH4) {
				if (Distancia(wk, cG7) - (c == blanques) < 2)
					own = 2;
				else
					own = 0;
			} //9
			if (own == unknown) {
				if ((Distancia(wk, p) < 3 && bk != cH4))
					own = 2; //45
				if (own == unknown) {
					if (Distancia(wk, cG5) < Distancia(bk, cG5) - (c == negres))
						own = 2; //13
					else {
						if (bk == cF7) //42
							own = 0;
						if (bk == cF6 && Distancia(wk, cG5) - (c == blanques) > 1 && Distancia(wk, cG7) - (c == blanques) > 1) //894247
							own = 0;  //40
						if (bk == cE7)
							own = 0; //37
						if (bk == cF8 && Distancia(wk, cF4) > Distancia(bk, cG5) - (c == negres))
							own = 0; //49
						if (own == unknown && c == blanques)
							own = 2; //130
					}
				}
			}
		}
	}

	if (own == unknown && p == cH6 && b == cH7 && (Distancia(bk, cG6) > 2 || bk == cE4 || wk == cH4 || Distancia(wk, p) <= Distancia(bk, cG6) - (c == negres)) && Distancia(bk, cF8) > 2)
		own = 2;  //124

	if (own == unknown)
		return kbpk_taules_auxiliar(ss);
	if (own == 0)
		return true;
	if (own == 2)
		return valor_victoria_coneguda_base_avaluacio;
	return false;
}


int eval_KRKP_llarg(tss *ss) {
	int bk, wk, t, p, c, x;
	Estat_Analisis *e = ss->estat_actual;
	int color_torre = Ttorres(blanques) ? blanques : negres;
	wk = lsb(Trei(color_torre));
	bk = lsb(Trei(oponent(color_torre)));
	t = lsb(Ttorres(color_torre));
	p = lsb(Tpeons(oponent(color_torre)));

	c = ss->estat_actual->mou;
	if (color_torre == negres) {
		//passar a com si fossin blanques
		wk = (7 - ROW(wk)) * 8 + COL(wk);
		bk = (7 - ROW(bk)) * 8 + COL(bk);
		t = (7 - ROW(t)) * 8 + COL(t);
		p = (7 - ROW(p)) * 8 + COL(p);
		c = oponent(c);
	}

	if (COL(p) > 3) {
		wk = wk ^ 7; //a1->h1
		bk = bk ^ 7;
		t = t ^ 7;
		p = p ^ 7;
	}

	Bitboard atwk = AtacsRei[wk];
	Bitboard atbk = AtacsRei[bk];
	Bitboard atp = AtacsPeo[negres][p];
	Bitboard btw = Bit(t);
	Bitboard bbk = Bit(bk);
	Bitboard bwk = Bit(wk);
	Bitboard bbp = Bit(p);
	Bitboard pecesblanques = btw | bwk;
	Bitboard attorre = AtacsTorre(t, (Bitboard)(bwk | bbk | bbp));
	Bitboard atblanques = attorre | atwk;
	Bitboard atnegres = atbk | atp;
	Bitboard totespeces = pecesblanques | bbk | bbp;

	bool reinenescac = (atblanques & bbk);

	if (c == negres && (atbk & atblanques) == atbk && !(atp & btw)) {
		//mat
		if ((atblanques & bbk)
			&& !(ROW(bk) == ROW(t) && ROW(p) == ROW(bk) + 1) //que no es pugui tapar de l'escac amb el peó
			)
			return 4;
		else //rei negre ofegat, i peó bloquejat
			if (!(atblanques & bbk) && (pecesblanques & Bit(p - 8)))
				return 2;
	}

	//cas concret d'ofegat
	if (bk == 0 && wk == 2 && t == 9 && p == 11)
		return 2;

	//torre captura peó negre no defensat
	if ((c == blanques || (Distancia(bk, p) - (ROW(p) == 6) > 3 && !(atbk & btw) && !(ROW(p) == 1)))
		&& (attorre & bbp) && !(atbk & bbp)) {
		return 999;
		/*Moviment m = FerMoviment(t, p);
		fer_moviment(ss, m);
		ss->estat_actual++;
		calcular_atacs(ss);
		if ((atbk & atblanques) != atbk) //que no l'ofegui
			return 4;
		desfer_moviment(ss, ss->estat_actual->moviment_previ);*/
	}

	//rei captura peó negre no defensat
	if (c == blanques && (atwk & bbp) && !(atbk & bbp)) {
		return 999;
		/*Moviment m = FerMoviment(wk, p);
		fer_moviment(ss, m);
		ss->estat_actual++;
		calcular_atacs(ss);
		if ((atbk & atblanques) != atbk) { //que no l'ofegui 
			//que el rei negre no pugui capturar la torre
			if (!((atbk & btw) && !(AtacsRei[p] & btw)))
				return 4;
		}
		desfer_moviment(ss, ss->estat_actual->moviment_previ);*/
	}

	//rei negre pot capturar torre
	if ((atbk & btw) && !(atwk & btw)) {
		if (c == negres)
			//mirar resultat final kpk
			return 999;
		//peo fa escac i rei no pot defensar torre ni torre pot menjar peó
		if ((atp & bwk) && Distancia(wk, t) > 1 && !(attorre & bbp))
			//mirar resultat final kpk
			return 999;
	}

	//peo negre pot capturar torre
	if (c == negres && (atp & btw)) {
		//mirar resultat final kpk
		return 999;
	}

	//per fila: torre clava peó o fa escac amenaçant peó i el menjarà
	if (ROW(p) != 1 && ROW(p) == ROW(t) && ROW(t) == ROW(bk) && Distancia(bk, p) - (c == negres) > 1 && ROW(wk) != ROW(bk)) {
		return 4;
	}

	//peo clavat i se'l menjaran
	if (ROW(t) == ROW(bk) && ROW(p) == ROW(bk)){
		if (Distancia(bk, p) - (c == negres) > 1) {
			if ((bk == 8 && wk == 2) || (bk == 15 && wk == 5)) {
				if (bk == 8 && t == 9)
					return 4;
				else {
					if (bk == 15 && t == 14)
						return 4;
				}
				if (bk == 15 && p == 11 && t != 12)
					return 4;
				if (Distancia(t, p) > 1 && COL(t) != COL(wk))
					return 4; //podrà parar el peó anant a 1a amb la torre
				return 2; //taules perquè s'autoofegarà
			}
			else {
				//si rei entre mig de torre i rei negre, no se sap
				if (ROW(wk) != ROW(bk)) //mirar excepcions quan rei entre mig però guanya
					return 4;
			}
		}
		else {
			if (Distancia(wk, p) == 1 && ROW(t) != ROW(wk)) {
				if (Distancia(bk, 0) == 1 && p == 9 && c == negres)
					return 2; //rei s'ofega a a1
				else
					return 4;
			}
		}
	}


	//casos especials rei negre en un extrem i rei blanc a c3
	if (p == 9 && (bk == 8 || bk == 0 || bk == 1) && wk == 18 && ROW(t) > 1 && !(atblanques & bbk)
		&& (c == negres || COL(t) != 1)
		) {
		if (ROW(t) == 2 && t != 17 && c == negres) {
			if (t == 16 || bk == 1)
				return 2;
			else
				return 0;
		}
		else {
			if (t == 17 && bk == 1)
				return 4;
			else {
				if ((ROW(t) > 2 || COL(t) == 0) && c == blanques)
					return 4;
				else {
					if (bk == 1) {
						if (c == blanques)
							return 4;
						else {
							if (COL(t) == 0 || COL(t) == 3)
								return 2;
							else
								return 4;
						}
					}
					else
						return 2;
				}
			}
		}
	}

	//peó parat i sel menjarà
	if ((atblanques & Bit(p - 8)) && Distancia(bk, p) - (c == negres) > 2 && ROW(p) != 6) {
		if (bk == 0 && c == negres && wk == 2 && t == 9 && p == 19)
			return 2; //cas especial ofegat
		else {
			if (bk == 0 && wk == 16 && t == 9 && (p == 25 || (p == 33 && c == negres)))
				return 2; //cas especial ofegat
			else {
				if (bk == 7 && c == negres && wk == 5 && ROW(t) == 1 && ROW(p) == 2 && COL(t) == COL(wk) && COL(p) < 3)
					return 2; //cas especial ofegat
				else {
					if (bk == 56 && wk == 40 && p == 9 && c == negres)
						return 2; //cas especial ofegat
					else
						return 4;
				}
			}
		}
	}


	//cas especial
	if (bk == 0 && wk == 2 && p == 9) {
		if (ROW(t) == 0 || COL(t) == 1)
			return 4;
		else {
			if (ROW(t) == 1 || COL(t) == 2 || COL(t) == 4 || ROW(t) > 2)
				return 2; //ofegat o ha de donar torre per dama
			else
				return 0;
		}
	}

	//cas especial
	if (bk == 0 && wk == 2 && p == 9 + 8 && ((c == negres && ROW(t) == 1) || (c == blanques && t == 10))) {
		return 2;
	}

	//ofegat o mat
	if (c == negres && bk + 8 == p && !(atbk & ~(atblanques | Bit(p)))) {
		if (atblanques & Bit(bk))
			return 4; //mat
		else
			return 2;
	}

	//tisores rei i torre
	if ((atp & bwk) && (atp & Bit(t)) && (atbk & Bit(p)))
		return 2;

	//tisores rei i torre
	if (c == negres && (AtacsPeo[negres][p - 8] & bwk) && (AtacsPeo[negres][p - 8] & Bit(t)) && (atbk & Bit(p - 8)) &&
		!(atblanques & bbk))
		return 2;

	//tisores rei i torre
	if (c == negres && ROW(p) == 6 && (AtacsPeo[negres][p - 16] & bwk) && (AtacsPeo[negres][p - 16] & Bit(t)) && (atbk & Bit(p - 16)) &&
		!(atblanques & bbk)) {
		if (bk + 8 == p)
			return 4; //rein bloqueja peó
		else
			return 2;
	}


	//rei ha parat peó
	if (ROW(p) > ROW(wk)) {
		if (COL(wk) == COL(p) ||
			((COL(wk) - (c == blanques) == COL(p) || COL(wk) + (c == blanques) == COL(p))
			&& ((atwk & Columnes[COL(p)]) & ~atnegres)) //hi ha alguna casella no atacada pel negre davant del peo on pugui anar el rei blanc
			) {
			if (c == negres && COL(bk) == 0 && wk + 16 == bk && bk + 8 == p && COL(t) == 1)
				return 2; //ofegat
			else
				return 4;
		}
		else {
			//rei pararà peó
			if (ROW(bk) - (c == negres) > ROW(wk)
				&& ((atwk & Columnes[COL(p)]) & ~atnegres) //hi ha alguna casella no atacada pel negre davant del peo on pugui anar el rei blanc
				&& ROW(bk) != 2
				)
				return 4;
		}
	}

	//ofegat o coronarà
	if ((p == 8 || (p == 16 && c == negres) || (p == 19 && c == negres)) && bk == 0 && wk == 2 && COL(t) < 2) {
		if (p == 19) {
			if (t == 9)
				return 2;
			else
				return 4;
		}
		else {
			if (p == 16 && t == 1)
				return 4; //fent escac 
			else
				return 2;
		}
	}

	//cas especial taules ofegat
	if ((p == 17 || p == 25) && (bk == 0 || bk == 1) && wk == 16 && t == 9)
		return 2;

	//peo coronant i rein no en escac
	bool coronaambescac = (!!((PrecalculTorre[p - 8] | PrecalculAlfil[p - 8]) & bwk)) && bk != p - 8 && t != p - 8;

	if (c == negres && ROW(p) == 1 && !reinenescac)  {
		if (wk == p - 8)
			return 4; //reib bloquejant peo
		else {
			if (atwk & (Bit(p - 8))) { //reib atacant avanç p
				if (!(atbk & (Bit(p - 8)))) {
					if (atbk & btw) {
						if (attorre & Bit(bk))
							return 4;
						else {
							if ((attorre & Bit(p - 8)) || (COL(t) == COL(p) && COL(t) != COL(wk)))
								return 4;  //torre para coronació
							else
								return 2; //rei negre menjarà torre doncs quedarà sense defensar en desviar el reib amb coronació
						}
					}
					else {
						if (bk == 0 && p == 10 && ROW(t) == 1)
							return 2; //reib a c1 ofegarà amb torre a 2a
						else
							return 4;
					}
				}
				else {
					if (COL(t) == COL(p))
						return 4; //rei i torre evitant coronació
				}
			}
		}

		if (coronaambescac) {
			if (Distancia(wk, p - 8) == 1 && !(attorre & Bit(p - 8)))
				return 0;
			else {
				if ((COL(t) < COL(p) && COL(wk) > COL(p)) || (COL(t) > COL(p) && COL(wk) < COL(p))) {
					//no pot tapar de torre
					if ((ROW(t) == 1 || ROW(t) == 0) && ROW(bk) == 1) {
						//peo clavat o torre controlant coronació
						if (Distancia(wk, p - 8) == 1)
							return 4; //controla coronació amb torre i rei
						else {
							if ((attorre & Bit(p - 8)) && Distancia(wk, p - 8) == 2) {
								if (Distancia(bk, p - 8) == 1)
									return 2;
								else
									return 4; //arriba a controlar coronació amb rei i ja la controla amb torre
							}
							else
								return 2;
						}
					}
					else {
						if (t == 0 && c == negres && p == 10 && (Distancia(bk, p) < Distancia(wk, p) || (ROW(wk) == 0 && ROW(bk) == 3 && COL(bk) == COL(p) + 1)))
							return 2; //cas especial torre no pot guanyar un temps
						else {
							if ((attorre & Bit(p - 8))) {
								if (Distancia(bk, p - 8) - (c == negres) == 1) {
									if (atbk & (~atwk) & (FILA2 ^ Bit(p))) {
										if ((COL(wk) < COL(p) && COL(bk) < COL(p)) || (COL(wk) > COL(p) && COL(bk) > COL(p)))
											return 2; //rein pot anar a 2a
										else {
											if (Distancia(t, p - 8) == 2)
												return 2; //al baixa amb el rei toca la torre guanyant un temps
											else {
												if (Distancia(wk, p - 8) > 2)
													return 2; //reib no arriba a temps d'ajudar la torre a parar el peó
												else
													return 4; //si que arriba a temps
											}
										}
									}
									else {
										return 4;
									}
								}
								else {
									if ((t == 0 || t == 1) && c == negres && (p == 10 || p == 11) && (bk == 21 || bk == 22) && COL(wk) > COL(bk))
										return 2; //reib massa lluny
									else {
										if (t == 0 && c == negres && p == 10 && (bk == 25 || bk == 26) && COL(wk) >= 4 && wk != 20)
											return 2; //cas especial
										else {
											if (COL(wk) == 7 && COL(t) < COL(bk) && ROW(bk) < 4)
												return 2; //rein arriba a suportar peo
											else {
												if (c == negres && Distancia(bk, p + 8) == 1 && Distancia(wk, p - 8) > 2 && Distancia(t, p - 8) == 2)
													return 2; //rein arriba pq guanya un temps sobre torre
												else {
													if (ROW(wk) >= ROW(bk) && Distancia(bk, p) < Distancia(t, p)
														&& ((COL(wk) > COL(bk) && COL(bk) >= COL(p)) || (COL(wk) < COL(bk) && COL(bk) <= COL(p))))
														return 2;
													/*else
													return 4;*/
												}
											}
										}
									}
								}
							}
						}
						//return 0;
					}
				}
				else {
					if (ROW(bk) == 0) {
						if (ROW(wk) == 0 && Distancia(wk, t) < 3 && ROW(t) == 0)
							return 2;
						else {
							if (ROW(wk) == 0 && COL(t) == COL(wk) - 1) {
								if (Distancia(p, bk) > 2) {
									if (COL(bk) < COL(p))
										return 2; //bo
									/*else
									return 4; //rein no pot defensar coronació*/
								}
								/*else
								return 2;*/
							}
						}
					}
				}
			}
		}
	}

	//cas especial
	if (bk == 0 && p == 9 && wk == 16) {
		if (reinenescac) {
			if (ROW(t) == 0)
				return 2;
			else
				return 4;
		}
		else {
			if (c == negres) {
				if (COL(t) == 1)
					return 2;
				else
					return 0;
			}
			else {
				if (c == blanques && COL(t) == 0)
					return 4;
				else
					return 2;
			}
		}
	}

	//torre parant peó per columna davant peo
	if (COL(t) == COL(p) && ROW(t) < ROW(p)) {
		//cas especial taules
		if (ROW(p) == 1 && t == p - 8 && (bk == p + 8 || bk == p + 9) && wk == t + 2) {
			if (COL(p) < 3 && ((bk == p + 9 && c == blanques) || (bk == p + 8 && c == negres)))
				return 2;
			else
				return 4;
		}
		else {
			if (Distancia(wk, t) < 2)
				return 4;
			else {
				if (Distancia(wk, t) - (c == blanques) < Distancia(bk, t) - (c == negres))
					return 4;
				else {
					if (Distancia(bk, p) - (c == negres) > 1 || Distancia(t, p) < Distancia(bk, p) - (c == negres)) {
						if (!(c == negres && ROW(p) == 6 && ROW(bk) == 3))
							return 4;
					}
				}
			}
		}
	}


	//rei ajudarà coronació peo amb torre parant peó per columna
	if (ROW(p) == 1 && COL(t) == COL(p)) {
		//if (c == blanques && Distancia(wk, p - 8) < 2)
		if (c == blanques && (atwk & Bit(p + 1)) && !(atbk & Bit(p + 1)) && !(atbk & Bit(t)))
			return 4;
		else {
			if (c == blanques && COL(p) != 0 && (atwk & Bit(p - 1)) && !(atbk & Bit(p - 1)) && !(atbk & Bit(t)))
				return 4;
			else
				/*if (Distancia(bk, p - 8) - (c == negres) < 2)
				own = 2*/;
		}
	}

	//torre tallant accés del rein al peó
	if (ROW(p) > 2 && ROW(bk) > 4 && ROW(t) > ROW(p) && ROW(t) < ROW(bk)
		&& !(ROW(wk) == ROW(t) && (COL(t) > COL(wk) && COL(wk) > COL(p))) //que reib no estigui tallant "tall" de torre
		)
		return 4;

	//torre tallant accés del rein al peó per sota
	if (ROW(p) > 2 && ROW(bk) + (c == negres) < ROW(t) - 1 && ROW(t) < ROW(p) && ROW(t) > ROW(bk) + 1 && ROW(p) != 6
		&& !(ROW(wk) == ROW(t) && (COL(t) > COL(wk) && COL(wk) > COL(p))) //que reib no estigui tallant "tall" de torre
		)
		return 4;

	//torre tallant accés del rein al peó per sota
	if (COL(bk) < COL(t) && COL(t) <= COL(p) && ROW(t) < ROW(p)) {
		if (Distancia(bk, t) > 2 && Distancia(bk, p) - (c == negres) > 2 - (c == negres && COL(t) == COL(bk) + 1 && COL(t) < COL(p) - 1)) {
			return 4;
		}
		else {
			if (Distancia(wk, t) == 1)
				return 4;
			else {
				if (Distancia(bk, p) - (c == negres) > 2) {
					if (c == negres && (wk == 48 || wk == 56) && bk == 17) {
						if (t == 34 && p == 51)
							return 4;
						else
							return 2;
					}
					else
						return 4;
				}
				else {
					if (ROW(p) > ROW(wk) && bk == wk - 16) {
						if (p == 50 && wk == 40)
							; // own == 2;
						else {
							if (p == 51 && wk == 41)
								; //own == 2;
							else
								return 4;
						}
					}
				}
			}
		}
	}

	//reib impedeix rein a arribar al peo
	if (wk == bk + 16 && ROW(p) > ROW(wk)) {
		if (bk == 24 && p == 50 && (t == 9 || (c == negres && t == 19)))
			return 2;
		else {
			if (bk == 25 && p == 51 && (t == 10 || (c == negres && t == 20)))
				return 2;
			else {
				if (bk == 27 && p == 49 && (t == 10 || t == 16) && c == negres)
					return 2;
				else {
					if (bk == 28 && p == 50 && (t == 11 || t == 17) && c == negres)
						return 2;
					else {
						if (bk == 29 && p == 51 && (t == 12 || t == 18) && c == negres)
							return 2;
						else
							return 4;
					}
				}
			}
		}
	}

	//reib impedeix rein a arribar al peo
	if (ROW(bk) < ROW(wk) - 2 && ROW(p) > ROW(wk) &&
		((COL(p) > COL(wk) && COL(bk) <= COL(wk)) || (COL(p) < COL(wk) && COL(bk) >= COL(wk)))
		) {
		return 4;
	}

	//torre tallant accés del rein al peó per sota
	if (COL(bk) > COL(t) && COL(t) >= COL(p) && ROW(t) < ROW(p) && ROW(p) > 1 && !(ROW(p) == 6 && Distancia(bk, p - 16) - (c == negres) < 2)) {
		if (Distancia(bk, t) > 2 && Distancia(bk, p) - (c == negres) > 2 - (c == negres && COL(t) == COL(bk) - 1 && COL(t) > COL(p) - 1)) {
			return 4;
		}
		else {
			if (Distancia(wk, t) == 1 && (ROW(p) > 2 || Distancia(wk, p) < Distancia(t, p)))
				return 4;
		}
	}

	//torre podrà tallar accés rei a peo o guanyar peo
	if (c == blanques && (attorre & Bit(ROW(t) * 8 + COL(p)))
		&& !(atbk & Bit(ROW(t) * 8 + COL(p)))
		&& ROW(t) < ROW(p) && ROW(p) > 1 && !(ROW(p) == 6 && Distancia(bk, p - 16) - (c == negres) < 2)) {
		if (Distancia(bk, t) > 2 && Distancia(bk, p) > 2) {
			return 4;
		}
	}

	//torre pot menjar peó
	if (abs(COL(p) - COL(bk)) > 3 - (c == blanques && COL(wk) != COL(p)) && ROW(p) - (c == negres) > 1
		&& !(ROW(wk) == ROW(t) && (COL(t) > COL(wk) && COL(wk) > COL(p)))) //que reib no estigui tallant "tall" de torre
		return 4;

	//cas especial ofegat
	if (wk == 40 && (bk == 57 || bk == 56) && t == 49 && (p == 9 || (p == 17 && c == negres && bk == 56)))
		return 2;

	//cas especial ofegat
	if (wk == 5 && bk == 7 && ROW(t) == 1 && ROW(p) == 1) {
		if (c == negres)
			return 0;
		else {
			if (abs(COL(t) - COL(p)) == 1) {
				if (p == 11) {
					if (t == 12)
						return 2;
					else
						return 4;
				}
				else
					return 2;
			}
			else {
				if (COL(t) == COL(wk)) {
					if (p == 11)
						return 4;
					else
						return 2;
				}
				else
					return 4;
			}
		}
	}

	//torre fent escac i guanya peó
	if (reinenescac && (attorre & Bit(p)) &&
		(Distancia(bk, p) > 2 || (Distancia(wk, t) < 3) && Distancia(wk, p) < 3 && ROW(wk) <= ROW(p))) {
		//excepcio taules
		if (wk == 0 && t == 8 && (bk == 16 || bk == 24) && p == 10)
			return 2;
		else {
			if ((wk == 0 || wk == 8) && t == 9 && (bk == 17 || bk == 25) && p == 10)
				return 2; //excepcio taules
			else {
				if (wk < 2 && t == 9 && (bk == 17 || bk == 25) && p == 11)
					return 2; //excepcio taules
				else {
					if (wk == 2 && t == 9 && (bk == 17 || bk == 25) && p == 8)
						return 2; //excepcio taules
					else {
						if ((wk == 1 || wk == 9) && t == 10 && (bk == 18 || bk == 26) && p == 11)
							return 2; //excepcio taules
						else {
							if (wk == 2 && t == 10 && (bk == 18 || bk == 26) && p == 8)
								return 2; //excepcio taules
							else {
								if ((wk == 3 || wk == 11) && t == 10 && (bk == 18 || bk == 26) && p == 9)
									return 2; //excepcio taules
								else {
									if (wk == 3 && t == 11 && (bk == 19 || bk == 27) && p == 9)
										return 2; //excepcio taules
									else {
										if ((wk == 4 || wk == 12) && t == 11 && (bk == 19 || bk == 27) && p == 10)
											return 2; //excepcio taules
										else {
											if (wk == 4 && t == 12 && (bk == 20 || bk == 28) && p == 10)
												return 2; //excepcio taules
											else {
												if ((wk == 5 || wk == 13) && t == 12 && (bk == 20 || bk == 28) && p == 11)
													return 2; //excepcio taules
												else {
													if (wk == 5 && t == 13 && (bk == 21 || bk == 29) && p == 11)
														return 2; //excepcio taules
													else
														return 4;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	//peo corona
	if (Entre[p - 8][wk] & Bit(bk))
		coronaambescac = false;
	if (ROW(p) == 1 && !(atblanques & Bit(p - 8))) {
		if (c == negres) {
			if (bk != p - 8) {
				if (ROW(bk) == 0 && (attorre & FILA1 & atwk)) {
					if (COL(t) - COL(wk) > 2) {
						if (COL(bk) > COL(p))
							return 2;
						else
							return 0;
					}
					else {
						if (ROW(wk) == 0) {
							if (ROW(t) == 0) {
								if (COL(bk) > COL(t))
									return 4;
								else
									return 2;
							}
						}
						else {
							if (abs(COL(p) - COL(bk)) < 3)
								return 2;
							else {
								if (COL(bk) > COL(p)) {
									if (ROW(t) == 0 && abs(COL(p) - COL(bk)) == 3)
										return 2;
								}
								/*	else
								return 2;*/
							}
						}
					}
				}
				else {
					if (!reinenescac && coronaambescac) {
						if (COL(t) != COL(p)) {
							if (ROW(p) == ROW(bk) && ROW(bk) == ROW(t))
								return 2;
							else
								return 0;
						}
					}

				}
			}
		}
	}

	//torre guanya peo
	if ((attorre & Bit(p)))
	{
		if (Distancia(bk, p) + (c == blanques) > 3 - !(Distancia(bk, p - 8) == 2)
			&& (ROW(p) > 1 || c == blanques) && (ROW(p) != 6 || Distancia(bk, p) + (c == blanques) > 4))
			return 4;
	}

	//cas especial, fa mat quan negre corona
	if (bk == 0 && p == 16 && ROW(t) == 1 && Distancia(wk, p) - (c == blanques) - (wk == 19) < 2 + (ROW(wk) != 0 && ROW(wk) < 4))
		return 4;

	//cas especial, fa mat quan negre corona
	if (bk == 16 && p == 17 && COL(t) == 1 &&
		(wk == 33 || (Distancia(wk, 9) - (c == blanques) + (COL(wk) == 0 || (Distancia(bk, t) == 1 && Distancia(wk, t) > 1)) < 3))
		&& ROW(t) > ROW(p))
		return 4;

	//alguns casos coronació
	if (!reinenescac && ROW(p) == 2 && c == negres && Distancia(wk, COL(p)) > 2 && COL(t) == COL(wk) && abs(ROW(t) - ROW(bk)) == 1 && abs(COL(bk) - COL(p)) <= 1
		&& ROW(bk) >= ROW(p)
		) {
		if (bk == 17 && ROW(t) > 2 && (wk == 12 || wk == 13 || wk == 14 || wk == 15 || (wk >= 20 && ROW(t) > ROW(wk))))
			return 0; //no pot parar el peó
		else {
			if (bk == 18 && ROW(t) > 2 && (wk == 8 || wk == 13 || wk == 14 || wk == 15 || (wk >= 21 && ROW(t) > ROW(wk))))
				return 0;
			else {
				if (bk == 19 && ROW(t) > 2 && (wk == 13 || wk == 14 || wk == 15 || (wk >= 22 && ROW(t) > ROW(wk))))
					return 0;
				else {
					if (bk == 20 && ROW(t) > 2 && (wk == 8 || wk == 14 || wk == 15 || wk == 16 || (wk >= 23 && ROW(t) > ROW(wk))))
						return 0;
					/*else
					return 2;*/
				}
			}
		}

	}

	//menja peo
	if ((atwk & Bit(p)) && (attorre & Bit(p)) && (bk == p + 8 || bk == p - 8 || c == blanques)) {
		if (bk == 0 && p == 9) {
			if (COL(t) == 1)
				return 4; //farà mate
			else
				return 2;
		}
		else {
			if (bk == 1 && p == 9 && c == negres && COL(t) != 0)
				return 2; //ofegarà
			else
				return 4;
		}
	}

	if (c == negres && ss->tau.c[p - 8] != CasellaBuida && !(atbk & (~(atblanques | Bit(p))))) {
		return 4; //mat o guanya
	}
	else {
		if (p == 8 && bk == 0 && wk == 2 /*&& (COL(t) == 1 || ROW(t) == 1 || COL(t) == 0)*/) {
			if (c == blanques && ROW(t) == 0)
				return 4; //mat
			else
				return 2; //ofegat o taules
		}
		else {
			if (p == 16 && bk == 0 && wk == 2 /*&& (ROW(t) == 1 || COL(t) == 1 || COL(t) == 0) && c == negres*/) {
				if (ROW(t) == 0 || c == blanques)
					return 4;
				else
					return 2;
			}
		}
	}


	//rei surt de casella 0 i vol coronar
	if (bk == 0 && p == 8) {
		if (c == negres) {
			if (ROW(t) == 2 &&
				((!(atwk & (Bit(18) | Bit(17) | Bit(10)))) || (wk == 27))
				) { //si reib no defensa casella on torre pugui fer escac, negre corona i guanya 
				if (COL(t) == 0 || (COL(t) != 2 && COL(t) != COL(wk)))
					return 2; //txp o escac i txp
				else {
					if (ROW(wk) != 1 && COL(t) > 2)
						return 2;
					else
						return 0;
				}
			}
			else {
				if (wk == 16 || wk == 17 || wk == 18) {
					if (wk == 18 && (COL(t) == 0 || (COL(t) == 2 && ROW(t) > 2)))
						return 2;
					else
						return 4; //farà mat
				}
				else {
					if (COL(t) > 2 && (wk == 24 || wk == 25 || (wk == 26 && ROW(t) == 1)))
						return 4;
					else {
						if (COL(wk) >= 2)
							return 2;
						else {
							if (COL(t) == 2 && ROW(t) > 3 && wk == t - 1)
								return 0;
							else
								return 2;
						}
					}

				}
			}
		}
		else { //c == blanques
			if (((t == 9 || t == 17) && (wk == t + 1 || wk == 10)) || (ROW(wk) == 0 && t == 9))
				return 2;
			else {
				if (COL(wk) > 3 || ROW(wk) > 4)
					return 2;
				else {
					if ((Distancia(wk, 10) < 2 && ROW(t) == 0) || (wk == 10))
						return 4; //mat
					else {
						if ((COL(t) > 2 || t == 10) && Distancia(wk, 18) < 2)
							return 4;
						else {
							if (wk == 16 || wk == 17 || wk == 18)
								return 4;
							else {
								if (Distancia(wk, 17) < 2 && (t != 17 || (ROW(wk) > 1 && wk != 26)))
									return 4;
								else {
									if (ROW(wk) == 4 && Distancia(wk, 17) < 3 && ROW(t) == 1 && t > 10)
										return 4;
									//return 2;
								}
							}
						}
					}
				}
			}

		}
	}

	//cas especial taules
	if (p == 17 && wk == 0 && t == 16 && Distancia(bk, 10) - (c == negres) < 1) {
		if ((bk == 2 || bk == 10) && c == negres)
			return 0;
		else
			return 2;
	}

	//cas especial taules
	if (p == 16 && c == negres) {
		if (bk == 24 || bk == 26) {
			if (wk == 2 && t == 1)
				return 2;
			else {
				if (wk == 18 && t == 17)
					return 2;
			}
		}
		else {
			if (bk == 32 && t == 25 && wk == 18)
				return 2;
			else {
				if (bk == 33 && t == 26 && wk == 18)
					return 2;
			}
		}
	}

	//cas especial taules
	if (p == 8 && t == 1 && bk == 24 && wk == 2) {
		return 2;
	}

	//casos especials
	if (ROW(p) == 1 && (atp & Bit(t)) && Distancia(bk, p + 8) < 2) {
		if ((atbk & Bit(p - 8)) && abs(wk - bk) != 2 && (bk == 8 || (wk != bk + 10 && wk != bk + 6)))
			return 2;
		else {
			if (atwk & Bit(p + 8)) {
				if (ROW(wk) == 2 || ROW(wk) == 1 || ROW(wk) == ROW(bk)) {
					return 4;
				}
				else
					return 2;
			}
			else {
				if (Distancia(wk, p + 1) < 2 && (Distancia(bk, p + 9) > 1 || (atwk & Bit(p + 9))))
					return 4;
				else {
					//return 2;
				}
			}
		}
	}

	if (ROW(bk) >= ROW(p) && ROW(p) > 1) {
		struct InfoEval ie;
		ie.rei[blanques] = lsb(bwk);
		ie.rei[negres] = lsb(bbk);
		int antmou = ss->estat_actual->mou;
		ss->estat_actual->mou = c;
		int kpk = -eval_KPK(ss, &ie, negres);
		ss->estat_actual->mou = antmou;
		if (kpk == 0) {
			return 4;
		}
	}

	//rei ha parat peó
	if (ROW(p) > ROW(wk) - (c == blanques) && ROW(bk) >= ROW(p) && Distancia(wk, p) - (c == blanques) < Distancia(bk, p) && ROW(p) - (c == negres) >= 2)
		return 4;

	//cas especial
	if (COL(bk) == 0 && COL(p) == 0 && ROW(p) > 2 - (c == negres) && (wk == 2 || wk == 10)) {
		if (p == 16 && t == 17 && wk == 10 & bk == 0 && c == negres)
			return 2;
		else
			return 4;
	}

	//rei parant peo
	if (COL(wk) == COL(p) + 1 && c == blanques && ROW(wk) < ROW(p)) {
		if (ROW(p) == 1 && p == wk + 7) {
			if (ROW(t) == 0)
				return 4;
			else {
				if (COL(t) == COL(wk) || COL(t) == COL(wk) + 2) {
					if (ROW(bk) == 0)
						return 2;  //clavarà dama amb torre
					else {
						if (COL(bk) == 0) {
							if (ROW(t) > ROW(bk) + 2)
								return 2; //fent escac canvia dama per torre
							else
								return 0;
						}
						else
							return 0;
					}
				}
				else {
					if (ROW(bk) == 0 && COL(bk) - COL(t) > 1)
						return 2; //farà escac per primera canviant dama x torre
					else {
						if (COL(bk) == 0) {
							if (ROW(t) > ROW(bk) + 2)
								return 2; //fent escac canvia dama per torre
							else
								return 0;
						}
						else
							return 0;
					}
				}
			}
		}
		else {
			if (bk == 8 && (t == 18 || t == 2) && p == 17)
				return 2;
			else {
				if (bk == 9 && (t == 16 || t == 19) && p == 18)
					return 2;
				else {
					if (bk == 10 && (t == 17 || t == 20) & p == 19)
						return 2;
					else
						return 4;
				}
			}
		}
	}

	//rei parant peo
	if (COL(wk) == COL(p) - 1 && c == blanques && ROW(wk) < ROW(p)) {
		if (ROW(p) == 1 && p == wk + 9) {
			if (ROW(t) == 0)
				return 4;
			else {
				if (COL(wk) == 0)
					return 0;
				else {
					if (COL(t) == COL(wk) || COL(t) == COL(wk) - 2) {
						if (ROW(bk) == 0)
							return 2;  //clavarà dama amb torre
						else {
							return 0;
						}
					}
					else {
						if (ROW(bk) == 0 && COL(t) - COL(bk) > 1)
							return 2; //farà escac per primera canviant dama x torre
						else {
							return 0;
						}
					}
				}
			}
		}
		else {
			if (bk == 10 && t == 19 && p == 17)
				return 2;
			else {
				if (bk == 11 && (t == 17 || t == 20) && p == 18)
					return 2;
				else {
					if (bk == 12 && (t == 18 || t == 21) & p == 19)
						return 2;
					else
						return 4;
				}
			}
		}
	}

	//rei i torre podran parar peo
	if (c == negres && COL(wk) == COL(p) + 1 && ROW(p) > ROW(wk) + 1) {
		if (ROW(p) > ROW(wk) + 2 || ROW(p) > 2) {
			if (t == 10 && p == 25 && bk == 0)
				return 2;
			else {
				if (t == 2 && p == 25 && bk == 8)
					return 2;
				else
					return 4;
			}
		}
		else {
			if (bk == 0 && p == 17 && t == 18)
				return 2;
			else {
				if (ROW(t) == 1 || ROW(t) == 0 || COL(t) == COL(p) || (COL(t) == COL(bk) && ROW(t) > 2)
					|| (COL(p) - COL(bk) >= 2 && (COL(t) > COL(p) + 1 || ROW(t) > ROW(p)))
					|| (COL(bk) - COL(p) > 1)
					)
					return 4;
				else {
					if (ROW(bk) > 1) {
						if (t == p + 1)
							return 2;
						else
							return 4;
					}
					else
						if (ROW(bk) == 0) {
							if (COL(bk) + 1 == COL(p)) {
								if (COL(t) == COL(bk) - 1)
									return 0;
								else {
									if (COL(t) == COL(wk) + 1) {
										if (ROW(t) == 2)
											return 0;
										else {
											if (COL(bk) == 0)
												return 2;
											else
												return 0;
										}
									}
									else {
										if (COL(t) == COL(wk) + 2 || COL(t) == COL(wk) || (ROW(t) > 2 && COL(bk) == 0) || COL(bk) - COL(t) > 1)
											return 2;
										else
											return 0;
									}
								}
							}
							else {
								if (bk == 0 && (t == 16 || t == 19))
									return 2;
								else {
									if (bk == 1 && (t == 17 || t == 20))
										return 2;
									else
										return 4;
								}
							}
						}
						else {
							//ROW(bk) == 1
							if (ROW(t) == 2) {
								if (COL(bk) + 2 == COL(p))
									return 2;
								else
									return 0;
							}
							else {
								if (ROW(t) == 3)
									return 0;
								else {
									if (COL(bk) == 0)
										return 2;
									else
										return 0;
								}
							}
						}
				}
			}
		}
	}

	//rei i torre podran parar peo
	if (c == negres && COL(wk) == COL(p) - 1 && ROW(p) > ROW(wk) + 1) {
		if (ROW(p) > ROW(wk) + 2 || ROW(p) > 2) {
			return 4;
		}
		else {
			if (ROW(t) == 1 || ROW(t) == 0 || COL(t) == COL(p) || (COL(t) == COL(bk) && ROW(t) > 2)
				|| (COL(bk) - COL(p) >= 2 && (COL(t) < COL(p) - 1 || ROW(t) > ROW(p)))
				|| (COL(p) - COL(bk) > 1)
				)
				return 4;
			else {
				if (ROW(bk) > 1) {
					if (t == p - 1)
						return 2;
					else
						return 4;
				}
				else
					if (ROW(bk) == 0) {
						if (COL(p) + 1 == COL(bk)) {
							if (COL(t) == COL(bk) + 1)
								return 0;
							else {
								if (COL(wk) == COL(t) + 1) {
									if (ROW(t) == 2)
										return 0;
									else
										return 0;
								}
								else {
									if (COL(wk) == 0)
										return 0;
									else {
										if (COL(wk) == COL(t) + 2 || COL(t) == COL(wk) || COL(t) - COL(bk) > 1)
											return 2;
										else
											return 0;
									}
								}
							}
						}
						else {
							if (bk == 3 && t == 19)
								return 2;
							else {
								if (bk == 4 && (t == 17 || t == 20))
									return 2;
								else {
									if (bk == 5 && (t == 18 || t == 21))
										return 2;
									else
										return 4;
								}
							}
						}
					}
					else {
						//ROW(bk) == 1
						if (ROW(t) == 3) {
							return 0;
						}
						else {
							if (COL(p) + 1 < COL(bk)) {
								if (t + 1 == p)
									return 2;
								else
									return 4;
							}
							else
								return 0;
						}
					}
			}
		}
	}

	//reib apropant-se al peó per parar-lo
	if (ROW(p) > 3 && ROW(wk) < ROW(p) && Distancia(wk, p) < ROW(p)) {
		if (c == blanques)
			return 4;
		else {
			if (Distancia(wk, COL(p)) < ROW(p) - (ROW(p) == 6)) {
				if (p == 35 && wk == 8 && bk == 10 && t == 0)
					return 2;
				else {
					if (COL(p) > 0 && ROW(p) == 4 && bk == p - 23 && wk == bk + 2 && t == wk - 8)
						return 2;
					else {
						if (COL(wk) == 0 && COL(p) == 3 && bk == 10) {
							if (p == 35 && wk == 24 && (t == 4 || t == 5 || t == 6 || t == 7 || t == 20 || t == 21 || t == 22 || t == 23))
								return 2;
							else
								return 4;
						}
						else {
							if (ROW(bk) == 1 && COL(p) + 1 == COL(bk)) {
								if (wk == bk + 18 && ROW(p) < 5 && COL(t) < COL(p) && (ROW(t) == 0 || ROW(t) == 2))
									return 2;
								else {
									if (COL(wk) > COL(bk) + 2 && ROW(wk) > 3 && COL(t) < COL(p) && (ROW(t) == 0 || ROW(t) == 2))
										return 2;
									else
										return 4;
								}
							}
							else {
								if (bk == 18 && wk == 24 && p == 35 && ((ROW(t) == 1 || ROW(t) == 3) && COL(t) > COL(p)))
									return 2;
								else {
									if (bk == 18 && p == 33 && wk == 28 && t == 8)
										return 2;
									else {
										if (!(COL(t) < COL(p) && (ROW(t) == 1 || ROW(t) == 3))) //torre mal posada
											return 4;
										else {
											if ((p == 41 || p == 49) && bk == 18 && wk == 37)
												return 2;
											else {
												if (p == 34 && wk == 29 && bk == 19)
													return 2;
												else {
													if ((p == 42 || p == 50) && wk == 38 && bk == 19)
														return 2;
													else {
														if (p == 35 && wk == 30 && bk == 20)
															return 2;
														else {
															if ((p == 43 || p == 51) && wk == 39 && bk == 20)
																return 2;
															else
																return 4;
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}


	//reib apropant-se al peó per parar-lo
	if (ROW(p) > 3 && Distancia(wk, COL(p)) - (c == blanques) - reinenescac < 3)
		return 4;

	//reib apropant - se al peó per parar-lo 
	if (ROW(p) > 3 && ROW(wk) == 0 &&
		((ROW(t) == 0 && ROW(p) > 4 && COL(wk) - (c == blanques) < 6)
		|| (Distancia(wk, COL(p)) - (c == blanques) - reinenescac < 5)
		)
		) {
		if (wk == 5 && p == 33 && ((bk == 18 && t == 8) || (bk == 10 && (t == 0 || t == 10 || t == 16)) || (bk == 0 && t == 10) || (bk == 8 && (t == 2 || t == 18))) && c == negres)
			return 2;
		else {
			if (wk == 4 && p == 32 && ((bk == 1 && t == 11) || (bk == 9 && (t == 3 || t == 19))) && c == negres)
				return 2;
			else {
				if (wk == 6 && p == 34 && ((bk == 19 && t == 9) || (bk == 17 && t == 11) || (bk == 11 && (t == 1 || t == 17)) || (bk == 1 && t == 11) || (bk == 9 && (t == 3 || t == 19))) && c == negres)
					return 2;
				else {
					if (wk == 7 && p == 35 && ((bk == 20 && t == 10) || (bk == 18 && t == 12) || (bk == 12 && (t == 2 || t == 18)) || (bk == 2 && t == 12) || (bk == 10 && (t == 4 || t == 20))) && c == negres)
						return 2;
					else {
						return 4;
					}
				}
			}
		}
	}

	//torre amenaça peó i se'l menjarà
	if (COL(t) == COL(p) && Distancia(bk, p) - (c == negres) > 1 && (attorre & Bit(p))) {
		if (!(c == negres && ROW(p) == 6 && (p - bk == 25 || p - bk == 23))) { //que avançant dos caselles no quedi defensat
			if (c == negres) {
				if (wk == 57 && bk == 24 && p == 48 && t == 56)
					return 2;
				else {
					if ((wk == 61 || wk == 62 || wk == 63) && bk == 25 && p == 49 && t == 57)
						return 2;
					else {
						if ((wk == 62 || wk == 63) && bk == 26 && p == 50 && t == 58)
							return 2;
						else {
							if (wk == 63 && bk == 27 && p == 51 && t == 59)
								return 2;
							else {
								if (wk == 40 && bk == 56 && p == 9 && (t == 17 || t == 25 || t == 33 || t == 41))
									return 2;
								else
									return 4;
							}
						}
					}
				}
			}
			else
				return 4;
		}
	}

	//peo retrassat
	if (ROW(p) + (c == blanques) > 3 && Distancia(bk, p) - (c == negres) > 4) {
		if (attorre & AtacsTorre(p + 8, totespeces)) //torre podrà anar davant peó i menjar-lo
			return 4;
		else {
			if ((attorre & AtacsTorre(COL(p), totespeces)) && Distancia(bk, COL(p)) + (c == blanques) > 2) //torre podrà anar davant peó i menjar-lo
				return 4;
		}
	}

	//peo retrassat a columna A
	if (COL(p) == 0 && ROW(p) > 3) {
		if (Distancia(wk, COL(p)) - (c == blanques) - (ROW(bk) == 0) < 4) {
			if (wk == 36 && bk == 17 && t == 10)
				return 2;
			else {
				if (wk == 4 && bk == 1 && t == 11 && p == 32)
					return 2;
				else
					return 4;
			}
		}
		else {
			/*if (Distancia(wk, COL(p)) > 6)
			return 2;*/
		}
	}

	//cas especial mat
	//5310956
	if (!reinenescac && bk == 7 && (wk == 13 || wk == 22)) {
		if (c == blanques)
			return 4;
		else {
			if (wk == 13) {
				if (ROW(t) == 1) {
					if (t == 12)
						return 2;
					else
						return 0;
				}
				else {
					if (p == 11 && (t == 18 || t == 20)) {
						return 2; //tisores de caball
					}
					else
						if (ROW(p) == 1) {
							if ((attorre & COLUMNAH) & PrecalculAlfil[p - 8]) { //dama al coronar evitarà mat per columna H
								if (COL(t) == 4)
									return 2; //escac a e1
								else
									return 0; //dama evita mat
							}
							else
								return 4; //mat
						}
						else
							return 4; //mat
				}
			}
			else {
				//wk == 22
				if (ROW(p) == 1)
					return 0;
				else {
					if (ROW(p) == 1) {
						if ((attorre & COLUMNAH) & PrecalculAlfil[p - 8]) { //dama al coronar evitarà mat per columna H
							return 0; //dama evita mat
						}
						else
							return 4; //mat
					}
					else
						return 4; //mat
				}
			}
		}
	}

	//cas especial mat
	if (bk == 63 && !reinenescac && (wk == 46 || wk == 53)) {
		if (c == blanques)
			return 4;
		else {
			if (wk == 46) {
				if (p == 9)
					return 0;
				else {
					if (COL(t) == 6 && ROW(p) == 1)
						return 0;
					else
						return 4;
				}
			}
			else
			{ //wk == 53
				if (ROW(t) == 6 && ROW(p) == 1)
					return 0;
				else {
					if (p == 9)
						return 2; //tapa escac amb dama
					else {
						if (ROW(p) == 1) {
							if ((attorre & COLUMNAH) & PrecalculAlfil[p - 8]) { //dama al coronar evitarà mat per columna H
								return 0; //dama evita mat
							}
							else
								return 4; //mat
						}
						else
							return 4; //mat
					}
				}
			}
		}
	}

	if (ROW(p) == 1 && ROW(bk) == 0 && c == negres && !(atblanques & Bit(p - 8)) && bk != p - 8 && !reinenescac) {
		Bitboard b = (attorre & atwk) & FILA1; //casella on torre pot anar a 1a defensat per rei
		bool torreatacapeo = attorre & Bit(p);
		if (b) {
			if (ROW(wk) == 1) {
				if (COL(t) > COL(bk) && COL(t) > COL(p)) {
					if (COL(bk) > COL(p) + 3 || (COL(bk) > COL(p) + 1 && torreatacapeo))
						return 4;
					else
						return 2;
				}
				else {
					if (COL(t) < COL(bk) && COL(t) < COL(p)) {
						return 2;
					}
					else
						if (COL(t) > COL(p) && COL(bk) == COL(t) + 1)
							return 0;
				}
			}
		}
	}

	//corona i potser guanya
	if (c == negres && !reinenescac && ROW(p) == 1 && ROW(t) != 0 && COL(t) < COL(p) && bk != p - 8) {
		if (ROW(t) == ROW(bk)) {
			if (ROW(wk) == ROW(t)) {
				if (COL(wk) < COL(p) && COL(t) < COL(wk))
					return 0;
				else {
					if (Distancia(bk, p) > 2) {
						if (ROW(wk) == ROW(bk) && wk < bk && p < wk)
							return 0;
						else
							return 4;
					}
					else {
						return 2;
					}
				}
			}
			else {
				return 2;
			}
		}
		else {
			if (COL(p) == COL(wk)) {
				//aquí COL(bk) == COL(p))
				if (ROW(t) < ROW(bk) || ROW(t) >= ROW(wk))
					return 0;
				else {
					if (ROW(wk) > ROW(t) + 1) {
						if (ROW(t) > ROW(bk) + 1)
							return 2;
						else
							return 0;
					}
					else {
						if (Distancia(bk, p) > 3)
							return 4;
						else
							return 2;
					}
				}
			}
			else {
				if (COL(bk) == 0 && wk == bk + 2) { //casos especials mat
					if (abs(ROW(t) - ROW(bk)) == 1)
						return 0;
					else {
						if ((p == 11 && wk == 18) || (p == 11 && wk == 34 && ROW(t) < ROW(bk)))
							return 2;
						else {
							if (p == 11 && (bk == 40 || bk == 48 || bk == 56) && ROW(t) == 3)
								return 0;
							else
								return 4;
						}
					}
				}
				else {
					//return 0;
					if (COL(p) == 2 && COL(bk) < 2 && ROW(t) > ROW(bk) && ROW(wk) == ROW(t) + 1) {
						if (ROW(t) < 3)
							return 0;
						else {
							if (COL(wk) == 0)
								return 0;
							else {
								if (COL(wk) > 1)
									return 0;
								else
									return 2;
							}

						}
					}
					else {
						if (p == 11 && COL(t) == 1 && wk == 0)
							return 2;
						else {
							if (wk == 8 && bk == 2) {
								if (ROW(t) == 2)
									return 0;
								else
									return 2;
							}
							else {
								if (bk == p - 7 && wk == bk + 10) {
									if (ROW(t) < 3)
										return 0;
									else
										return 2;
								}
								else
								{
									if (COL(t) == 0 && bk == 2 && p == 11 && !(COL(wk) == 0 && wk < t) && !(wk == t + 1) && !(wk == t + 2 && wk != 18))
										return 2;
									else {
										if (p == 10 && (wk == 0 || wk == 8) && COL(t) == 1 && ROW(bk) == 0)
											return 2;
										else {
											if (p == 11 && (wk == 1 || wk == 9) && COL(t) == 2 && ROW(bk) == 0)
												return 2;
											else {
												if (wk == 8 && bk == 10) {
													if (ROW(t) == 2 || ROW(t) == 3)
														return 0;
													else
														return 2;
												}
												else {
													if (wk == bk + 2 && COL(bk) == COL(p) + 1) {
														if (ROW(t) <= ROW(bk) + 2 || ROW(wk) == 0)
															return 0;
														else
															return 2;
													}
													else {
														if (COL(bk) == 7 && wk == bk - 2 && abs(ROW(t) - ROW(bk)) > 1 &&
															!((attorre & COLUMNAH) & PrecalculAlfil[p - 8])  //dama al coronar evitarà mat per columna H
															) {
															if (ROW(t) > ROW(bk) && (PrecalculAlfil[p - 8] & Bit(bk + 8)))
																return 2; //tapa de dama
															else {
																if (ROW(t) < ROW(bk) && (PrecalculAlfil[p - 8] & Bit(bk - 8)))
																	return 2; //tapa de dama
																else
																	return 4;
															}
														}
														else {
															if (COL(bk) == COL(p)) {
																if (ROW(t) > ROW(bk) + 2) {
																	if (Distancia(wk, p) < 3) {
																		if (Distancia(bk, p) < 3)
																			return 2;
																		else {
																			if (AtacsCaball[p - 8] & bwk) { //fa escac de caball al coronar 
																				if ((wk == 11 || wk == 18) && bk == 33)
																					return 4;
																				else
																					return 2;
																			}
																			else
																				return 4;
																		}
																	}
																	else {
																		if (Distancia(wk, p) < 3 && Distancia(bk, p) == 3)
																			return 4;
																		else {
																			if (ROW(t) == ROW(wk) && COL(wk) <= COL(p) && COL(t) < COL(wk))
																				return 0;
																			else {
																				if (bk == p + 24 && (wk == p + 3 || wk == p - 3))
																					return 4;
																				else
																					return 2;
																			}
																		}
																	}
																}
																else {

																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return 999;
}



