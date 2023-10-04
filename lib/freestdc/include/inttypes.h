#ifndef _FREESTDC_INTTYPES_H_
#define _FREESTDC_INTTYPES_H_

#include "stdint.h"

typedef struct _div_t {
	int quot;
	int rem;
}div_t;
typedef struct _ldiv_t {
	long quot;
	long rem;
}ldiv_t;
typedef struct _lldiv_t {
	long long quot;
	long long rem;
}lldiv_t;
typedef struct _imaxdiv_t {
	intmax_t quot;
	intmax_t rem;
}imaxdiv_t;

//
// fprintf format constants
//
#define PRId8 "d"
#define PRId16 "d"
#define PRId32 "d"
#define PRId64 "lld"
#define PRIdLEAST8 "hhd"
#define PRIdLEAST16 "hd"
#define PRIdLEAST32 "d"
#define PRIdLEAST64 "lld"
#define PRIdFAST8 "d"
#define PRIdFAST16 "d"
#define PRIdFAST32 "d"
#define PRIdFAST64 "lld"
#define PRIdMAX PRId64
#define PRIdPTR PRIdMAX

#define PRIi8 "i"
#define PRIi16 "i"
#define PRIi32 "i"
#define PRIi64 "lli"
#define PRIiLEAST8 "hhi"
#define PRIiLEAST16 "hi"
#define PRIiLEAST32 "i"
#define PRIiLEAST64 "lli"
#define PRIiFAST8 "i"
#define PRIiFAST16 "i"
#define PRIiFAST32 "i"
#define PRIiFAST64 "lli"
#define PRIiMAX PRIi64
#define PRIiPTR PRIiMAX

#define PRIu8 "u"
#define PRIu16 "u"
#define PRIu32 "u"
#define PRIu64 "llu"
#define PRIuLEAST8 "hhu"
#define PRIuLEAST16 "hu"
#define PRIuLEAST32 "u"
#define PRIuLEAST64 "llu"
#define PRIuFAST8 "u"
#define PRIuFAST16 "u"
#define PRIuFAST32 "u"
#define PRIuFAST64 "llu"
#define PRIuMAX PRIu64
#define PRIuPTR PRIuMAX

#define PRIo8 "o"
#define PRIo16 "o"
#define PRIo32 "o"
#define PRIo64 "llo"
#define PRIoLEAST8 "hho"
#define PRIoLEAST16 "ho"
#define PRIoLEAST32 "o"
#define PRIoLEAST64 "llo"
#define PRIoFAST8 "o"
#define PRIoFAST16 "o"
#define PRIoFAST32 "o"
#define PRIoFAST64 "llo"
#define PRIoMAX PRIo64
#define PRIoPTR PRIoMAX

#define PRIx8 "x"
#define PRIx16 "x"
#define PRIx32 "x"
#define PRIx64 "llx"
#define PRIxLEAST8 "hhx"
#define PRIxLEAST16 "hx"
#define PRIxLEAST32 "x"
#define PRIxLEAST64 "llx"
#define PRIxFAST8 "x"
#define PRIxFAST16 "x"
#define PRIxFAST32 "x"
#define PRIxFAST64 "llx"
#define PRIxMAX PRIx64
#define PRIxPTR PRIxMAX

#define PRIX8 "X"
#define PRIX16 "X"
#define PRIX32 "X"
#define PRIX64 "llX"
#define PRIXLEAST8 "hhX"
#define PRIXLEAST16 "hX"
#define PRIXLEAST32 "X"
#define PRIXLEAST64 "llX"
#define PRIXFAST8 "X"
#define PRIXFAST16 "X"
#define PRIXFAST32 "X"
#define PRIXFAST64 "llX"
#define PRIXMAX PRIX64
#define PRIXPTR PRIXMAX

//
// scanf format constants
//
#define SCNd8 "hhd"
#define SCNd16 "hd"
#define SCNd32 "d"
#define SCNd64 "lld"
#define SCNdLEAST8 "hhd"
#define SCNdLEAST16 "hd"
#define SCNdLEAST32 "d"
#define SCNdLEAST64 "lld"
#define SCNdFAST8 "hhd"
#define SCNdFAST16 "hd"
#define SCNdFAST32 "d"
#define SCNdFAST64 "lld"
#define SCNdMAX SCNd64
#define SCNdPTR SCNdMAX

#define SCNi8 "hhi"
#define SCNi16 "hi"
#define SCNi32 "i"
#define SCNi64 "lli"
#define SCNiLEAST8 "hhi"
#define SCNiLEAST16 "hi"
#define SCNiLEAST32 "i"
#define SCNiLEAST64 "lli"
#define SCNiFAST8 "hhi"
#define SCNiFAST16 "hi"
#define SCNiFAST32 "i"
#define SCNiFAST64 "lli"
#define SCNiMAX SCNi64
#define SCNiPTR SCNiMAX

#define SCNu8 "hhu"
#define SCNu16 "hu"
#define SCNu32 "u"
#define SCNu64 "llu"
#define SCNuLEAST8 "hhu"
#define SCNuLEAST16 "hu"
#define SCNuLEAST32 "u"
#define SCNuLEAST64 "llu"
#define SCNuFAST8 "hhu"
#define SCNuFAST16 "hu"
#define SCNuFAST32 "u"
#define SCNuFAST64 "llu"
#define SCNuMAX SCNu64
#define SCNuPTR SCNuMAX

#define SCNo8 "hho"
#define SCNo16 "ho"
#define SCNo32 "o"
#define SCNo64 "llo"
#define SCNoLEAST8 "hho"
#define SCNoLEAST16 "ho"
#define SCNoLEAST32 "o"
#define SCNoLEAST64 "llo"
#define SCNoFAST8 "hho"
#define SCNoFAST16 "ho"
#define SCNoFAST32 "o"
#define SCNoFAST64 "llo"
#define SCNoMAX SCNo64
#define SCNoPTR SCNoMAX

#define SCNx8 "hhx"
#define SCNx16 "hx"
#define SCNx32 "x"
#define SCNx64 "llx"
#define SCNxLEAST8 "hhx"
#define SCNxLEAST16 "hx"
#define SCNxLEAST32 "x"
#define SCNxLEAST64 "llx"
#define SCNxFAST8 "hhx"
#define SCNxFAST16 "hx"
#define SCNxFAST32 "x"
#define SCNxFAST64 "llx"
#define SCNxMAX SCNx64
#define SCNxPTR SCNxMAX

#define SCNX8 "hhX"
#define SCNX16 "hX"
#define SCNX32 "X"
#define SCNX64 "llX"
#define SCNXLEAST8 "hhX"
#define SCNXLEAST16 "hX"
#define SCNXLEAST32 "X"
#define SCNXLEAST64 "llX"
#define SCNXFAST8 "hhX"
#define SCNXFAST16 "hX"
#define SCNXFAST32 "X"
#define SCNXFAST64 "llX"
#define SCNXMAX SCNX64
#define SCNXPTR SCNXMAX

inline intmax_t imaxabs(intmax_t n) {
	return n < 0 ? -n : n;
}

inline imaxdiv_t imaxdiv(intmax_t x, intmax_t y) {
	imaxdiv_t d;
	d.quot = x / y, d.rem = x % y;
	return d;
}

intmax_t strtoimax(const char* nptr, char** endptr, int base);
uintmax_t strtoumax(const char* nptr, char** endptr, int base);

#endif
