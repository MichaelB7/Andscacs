#include "hash.h"
#include "mtwist.h"
#include "analisis.h"
#include "utils.h"
#include "debug.h"
#include "avaluacio.h"
#include <string.h>
#include <fstream>
#ifdef WINDOWS
#include <malloc.h>
#endif

#ifdef LINUX
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#endif

ClauHash ClauInici; //per no començar a 0
ClauHash ClauTorn;
ClauHash ClauPeca[16][64]; //fins el número del rei negre, que és el més gran
ClauHash ClauEnroc[16]; //combinacions de 4 bits
ClauHash ClauEP[8];
ClauHash ClauMovimentExclos; //s'usa per diferenciar els hash guardats dels moviments exclosos com a conseqüència del Singular extension search

ComptadorsHash memoriahashbytes = mbhashdefecte;
ComptadorsHash numentradeshash;
uint32_t HashMask;

struct thash *hash_;
void* memhash;
uint8_t edathash;
bool NeverClearHash = false;
char HashFile[1024];

uint32_t numaleatori = 82352;

#define __min(a,b)  (((a) < (b)) ? (a) : (b))

void inicialitza_claus_hash(uint32_t rands)
{
	int i;
	int j;

	mt_seed32new(rands);
	ClauTorn = mt_llrand();
	for (i = 0; i < 8; i++) ClauEP[i] = mt_llrand();
	for (i = 0; i < 16; i++) ClauEnroc[i] = mt_llrand();
	ClauMovimentExclos = mt_llrand();
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 64; j++) ClauPeca[i][j] = mt_llrand();
	}
	ClauInici = mt_llrand();
}

void posar_hash_posicio_i_valor_peces(tss *ss){
	int i;
	ss->estat_actual->hash = ClauInici;
	ss->estat_actual->hashpeons = ClauInici;
	ss->estat_actual->valorpeces[blanques] = 0;
	ss->estat_actual->valorpeces[negres] = 0;
	if (ss->estat_actual->mou) {
		ss->estat_actual->hash ^= ClauTorn;
	}
	ss->estat_actual->hash ^= ClauEnroc[ss->estat_actual->enroc];
	if (ss->estat_actual->casellapeoalpas) ss->estat_actual->hash ^= ClauEP[COL(ss->estat_actual->casellapeoalpas)];
	for (i = 0; i < 64; i++) if (ss->tau.c[i] != CasellaBuida) {
		ss->estat_actual->hash ^= ClauPeca[ss->tau.c[i]][i];
		if (ss->tau.c[i] == ReiBlanc || ss->tau.c[i] == ReiNegre || ss->tau.c[i] == PeoBlanc || ss->tau.c[i] == PeoNegre)
			ss->estat_actual->hashpeons ^= ClauPeca[ss->tau.c[i]][i];
		ss->estat_actual->valorpeces[Color(ss->tau.c[i])] += valorpeca[ss->tau.c[i]];
	}
}

void inicialitza_hash()
{
	ComptadorsHash xx;
	struct thash unahash; //només per tenir sizeof

	xx = memoriahashbytes;

#ifdef LINUX
	if (memhash)
		free(memhash);
	memhash = calloc(sizeof(unahash), xx);
	//	memhash = aligned_alloc(sizeof(unahash), xx);
#endif
#ifdef WINDOWS
	if (memhash)
		_aligned_free(memhash);
	memhash = _aligned_malloc(xx, sizeof(unahash));
#endif

	if (!memhash)
	{
		exit(EXIT_FAILURE);
	}

	hash_ = (thash*)memhash;

	numentradeshash = memoriahashbytes / sizeof(unahash);
	HashMask = numentradeshash - 1;

	buidar_hash();
	buidar_hashpeces();
	edathash = 0;
}

void buidar_hash() {
	if (NeverClearHash)
		return;
	edathash = 0;
	memset(hash_, 0, memoriahashbytes);
}

struct thash* buscar_hash(ClauHash hashposicio) {
	thash* tte = primera_entrada(hashposicio);
	ClauGuardaHash key32 = hashposicio >> 32;
	for (unsigned i = 0; i < NumEntradesHashGrup; i++, tte++) //index dins de hash
		if ((tte->w1 & 0xFFFFFFFF) == key32) {
			return tte;
		}
	return NULL;
}

bool usar_entrada(struct thash *auxhashb, ClauGuardaHash key32, Moviment *m){
	//si no ha trobat la clau de la posició
	if ((auxhashb->w1 & 0xFFFFFFFF) != key32) {
		if (!(auxhashb->w1 & 0xFFFFFFFF)) {//entrada no usada
			return true;
		}
	}
	else {
		if (*m == JugadaInvalida)
			if (movimenthash_p(auxhashb) != JugadaInvalida)
				*m = movimenthash_p(auxhashb); //preservar un moviment existent
		return true;
	}
	return false;
}

void guardar_hash(int score, int n, int nivell, ClauHash hashposicio, uint8_t tt_flag, Moviment moviment, Moviment segonmoviment, Valor avaluacio) {
	struct thash *h1;
	struct thash *h2;
	struct thash *h3;
	struct thash *h4;
	struct thash *auxhashrep;
	//ajustar puntuació de mat
	if (score > maxbeta - 300)
		score += n - 1;
	else if (score < -maxbeta + 300)
		score -= n - 1;
	ClauGuardaHash key32 = hashposicio >> 32;
	h1 = primera_entrada(hashposicio);
	if (usar_entrada(h1, key32, &moviment)) {
		auxhashrep = h1;
		goto fi;
	}
	h2 = h1 + 1;
	if (usar_entrada(h2, key32, &moviment)) {
		auxhashrep = h2;
		goto fi;
	}
	h3 = h2 + 1;
	if (usar_entrada(h3, key32, &moviment)) {
		auxhashrep = h3;
		goto fi;
	}
	h4 = h3 + 1;
	if (usar_entrada(h4, key32, &moviment)) {
		auxhashrep = h4;
		goto fi;
	}
	int a1, a2, a3, a4;
	a1 = (int)h1->edat << 3;
	a2 = (int)h2->edat << 3;
	a3 = (int)h3->edat << 3;
	a4 = (int)h4->edat << 3;

	a1 += h1->tipuspuntuaciohash == tt_exacte ? 1 : 0;
	a2 += h2->tipuspuntuaciohash == tt_exacte ? 1 : 0;
	a3 += h3->tipuspuntuaciohash == tt_exacte ? 1 : 0;
	a4 += h4->tipuspuntuaciohash == tt_exacte ? 1 : 0;

	a1 += (int)h1->depth;
	a2 += (int)h2->depth;
	a3 += (int)h3->depth;
	a4 += (int)h4->depth;

	int i;
	if (a2 <= a1) {
		auxhashrep = h2;
		i = a2;
	}
	else {
		i = a1;
		auxhashrep = h1;
	}
	if (a3 <= i) {
		auxhashrep = h3;
		i = a3;
	}
	if (a4 <= i)
		auxhashrep = h4;

fi:
	auxhashrep->w1 = key32 | ((uint64_t)moviment << 32) | ((uint64_t)avaluacio << 48);
	auxhashrep->depth = nivell;
	auxhashrep->score = score;
	auxhashrep->tipuspuntuaciohash = tt_flag;
	auxhashrep->edat = edathash;
	auxhashrep->segonmoviment = segonmoviment;
	return;
}

bool write_hash_binary()
{
	std::ofstream b_stream(HashFile,
		std::fstream::out | std::fstream::binary);
	if (b_stream)
	{
		for (long long i = 0; i < memoriahashbytes + CACHE_LINE_SIZE - 1; i += (1 << 30)) { //1GB
			long long j = __min((1 << 30), (memoriahashbytes + CACHE_LINE_SIZE - 1) - i);
			b_stream.write(reinterpret_cast<char const *>(memhash) + i, j);
		}
		return (b_stream.good());
	}
	return false;
}

#ifdef WINDOWS
__int64 FileSize(const wchar_t* name)
{
	__stat64 buf;
	if (_wstat64(name, &buf) != 0)
		return -1;

	return buf.st_size;
}

#endif

void read_hash_binary() {

#ifdef WINDOWS
	//ifstream unable to handle more than 4GB
	//char* to wchar_t*
	//https://stackoverflow.com/questions/8032080/how-to-convert-char-to-wchar-t
	size_t length = strlen(HashFile);
	wchar_t text_wchar[200];
	mbstowcs_s(&length, text_wchar, HashFile, length);
	long long size = FileSize(text_wchar);
	size -= (CACHE_LINE_SIZE - 1);

	std::ifstream file(HashFile, std::ios::binary);
#endif

#ifdef LINUX
	//file size: https://stackoverflow.com/questions/2409504/using-c-filestreams-fstream-how-can-you-determine-the-size-of-a-file
	std::ifstream file;
	file.open(HashFile, std::ios::in | std::ios::binary);
	file.ignore(std::numeric_limits<std::streamsize>::max());
	std::streamsize size = file.gcount();
	file.clear();   //  Since ignore will have set eof.
	size -= (CACHE_LINE_SIZE - 1);
#endif
	file.seekg(0, std::ios::beg);
	memoriahashbytes = size;
	inicialitza_hash();
	file.read(reinterpret_cast<char *>(memhash), size + CACHE_LINE_SIZE - 1);
}

//////////////
//Hash peons//
//////////////
tHashPeons HashPeons[(MBHashPeons << 20) / sizeof(tHashPeons)];
int NumHashPeons = (MBHashPeons << 20) / sizeof(tHashPeons);
