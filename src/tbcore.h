/*
Copyright (c) 2011-2015 Ronald de Man
*/

#ifndef TBCORE_H
#define TBCORE_H

#ifndef _WIN32
#include <pthread.h>
#define SEP_CHAR ':'
#define FD int
#define FD_ERR -1
#else
#include <windows.h>
#define SEP_CHAR ';'
#define FD HANDLE
#define FD_ERR INVALID_HANDLE_VALUE
#endif

#ifdef TB_HAVE_THREADS
#ifndef _WIN32
#define LOCK_T pthread_mutex_t
#define LOCK_INIT(x) pthread_mutex_init(&(x), NULL)
#define LOCK(x) pthread_mutex_lock(&(x))
#define UNLOCK(x) pthread_mutex_unlock(&(x))
#else
#define LOCK_T HANDLE
#define LOCK_INIT(x) do { x = CreateMutex(NULL, FALSE, NULL); } while (0)
#define LOCK(x) WaitForSingleObject(x, INFINITE)
#define UNLOCK(x) ReleaseMutex(x)
#endif
#else       /* !TB_HAVE_THREADS */
#define LOCK_T          int
#define LOCK_INIT(x)    /* NOP */
#define LOCK(x)         /* NOP */
#define UNLOCK(x)       /* NOP */
#endif

#define WDLSUFFIX ".rtbw"
#define DTZSUFFIX ".rtbz"
#define WDLDIR "RTBWDIR"
#define DTZDIR "RTBZDIR"
#define TBPIECES 6

#define WDL_MAGIC 0x5d23e871
#define DTZ_MAGIC 0xa50c66d7

#define TBMAX_PIECE 254
#define TBMAX_PAWN 256
#define HSHMAX 5

#define TBHASHBITS 10

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned char ubyte;
typedef unsigned short ushort;

struct TBHashEntry;

#ifdef DECOMP64
typedef uint64 base_t;
#else
typedef uint32 base_t;
#endif

struct PairsData {
	char *indextable;
	ushort *sizetable;
	ubyte *data;
	ushort *offset;
	ubyte *symlen;
	ubyte *sympat;
	int blocksize;
	int idxbits;
	int min_len;
	base_t base[1]; // C++ complains about base[]...
};

struct TBEntry {
	char *data;
	uint64 key;
	uint64 mapping;
	ubyte ready;
	ubyte num;
	ubyte symmetric;
	ubyte has_pawns;
}
#ifdef __GNUC__
__attribute__((__may_alias__));
#else
;
#endif

struct TBEntry_piece {
	char *data;
	uint64 key;
	uint64 mapping;
	ubyte ready;
	ubyte num;
	ubyte symmetric;
	ubyte has_pawns;
	ubyte enc_type;
	struct PairsData *precomp[2];
	int factor[2][TBPIECES];
	ubyte pieces[2][TBPIECES];
	ubyte norm[2][TBPIECES];
};

struct TBEntry_pawn {
	char *data;
	uint64 key;
	uint64 mapping;
	ubyte ready;
	ubyte num;
	ubyte symmetric;
	ubyte has_pawns;
	ubyte pawns[2];
	struct {
		struct PairsData *precomp[2];
		int factor[2][TBPIECES];
		ubyte pieces[2][TBPIECES];
		ubyte norm[2][TBPIECES];
	} file[4];
};

struct DTZEntry_piece {
	char *data;
	uint64 key;
	uint64 mapping;
	ubyte ready;
	ubyte num;
	ubyte symmetric;
	ubyte has_pawns;
	ubyte enc_type;
	struct PairsData *precomp;
	int factor[TBPIECES];
	ubyte pieces[TBPIECES];
	ubyte norm[TBPIECES];
	ubyte flags; // accurate, mapped, side
	ushort map_idx[4];
	ubyte *map;
};

struct DTZEntry_pawn {
	char *data;
	uint64 key;
	uint64 mapping;
	ubyte ready;
	ubyte num;
	ubyte symmetric;
	ubyte has_pawns;
	ubyte pawns[2];
	struct {
		struct PairsData *precomp;
		int factor[TBPIECES];
		ubyte pieces[TBPIECES];
		ubyte norm[TBPIECES];
	} file[4];
	ubyte flags[4];
	ushort map_idx[4][4];
	ubyte *map;
};

struct TBHashEntry {
	uint64 key;
	struct TBEntry *ptr;
};

struct DTZTableEntry {
	uint64 key1;
	uint64 key2;
	struct TBEntry *entry;
};

extern int wdl_to_map[5];
extern ubyte pa_flags[5];

int pawn_file(struct TBEntry_pawn *ptr, int *pos);
void load_dtz_table(char *str, uint64 key1, uint64 key2);
void init_tablebases(const char *path);
int init_table_wdl(struct TBEntry *entry, char *str);
void init_indices(void);
void free_wdl_entry(struct TBEntry *entry);
void free_dtz_entry(struct TBEntry *entry);
#ifndef CONNECTED_KINGS
uint64 encode_piece(struct TBEntry_piece *ptr, ubyte *norm, int *pos, int *factor);
#else
uint64 encode_piece(struct TBEntry_piece *ptr, ubyte *norm, int *pos, int *factor);
#endif
uint64 encode_pawn(struct TBEntry_pawn *ptr, ubyte *norm, int *pos, int *factor);
ubyte decompress_pairs(struct PairsData *d, uint64 index);

#define TB_PAWN 1
#define TB_KNIGHT 2
#define TB_BISHOP 3
#define TB_ROOK 4
#define TB_QUEEN 5
#define TB_KING 6

#define TB_WPAWN TB_PAWN
#define TB_BPAWN (TB_PAWN | 8)

#define DTZ_ENTRIES 64

extern struct TBHashEntry TB_hash[1 << TBHASHBITS][HSHMAX];
extern struct DTZTableEntry DTZ_table[DTZ_ENTRIES];
#endif

