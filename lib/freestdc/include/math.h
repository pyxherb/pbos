#ifndef _FREESTDC_MATH_H_
#define _FREESTDC_MATH_H_

#include "inttypes.h"

static inline int abs(int n);
static inline long labs(long n);
static inline long long llabs(long long n);

static inline div_t div(int x, int y);
static inline ldiv_t ldiv(long x, long y);
static inline lldiv_t lldiv(long long x, long long y);

static inline int abs(int n) {
	return n < 0 ? -n : n;
}
static inline long labs(long n) {
	return n < 0 ? -n : n;
}
static inline long long llabs(long long n) {
	return n < 0 ? -n : n;
}

static inline div_t div(int x, int y) {
	div_t d;
	d.quot = x / y, d.rem = x % y;
	return d;
}
static inline ldiv_t ldiv(long x, long y) {
	ldiv_t d;
	d.quot = x / y, d.rem = x % y;
	return d;
}
static inline lldiv_t lldiv(long long x, long long y) {
	lldiv_t d;
	d.quot = x / y, d.rem = x % y;
	return d;
}

#endif
