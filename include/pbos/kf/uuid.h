#ifndef _PBOS_KF_UUID_H_
#define _PBOS_KF_UUID_H_

#include <stdint.h>

typedef struct _uuid_t {
	unsigned int a;
	unsigned short b;
	unsigned short c;
	unsigned short d;
	unsigned int e1;
	unsigned short e2;
} uuid_t;

#define UUID(_a, _b, _c, _d, _e) ((uuid_t){ \
	.a = 0x##_a,                            \
	.b = 0x##_b,                            \
	.c = 0x##_c,                            \
	.d = 0x##_d,                            \
	.e1 = ((uint64_t)0x##_e) >> 16,         \
	.e2 = ((uint64_t)0x##_e) & 0xffff })

#define uuid_eq(_a, _b)      \
	(((_a)->a == (_b)->a) &&      \
		((_a)->b == (_b)->b) &&   \
		((_a)->c == (_b)->c) &&   \
		((_a)->d == (_b)->d) &&   \
		((_a)->e1 == (_b)->e1) && \
		((_a)->e2 == (_b)->e2))
#define uuid_neq(_a, _b) (!uuid_eq(_a, _b))

static inline bool uuid_gt(const uuid_t* _a, const uuid_t* _b) {
	if(_a->a > _b->a)
		return true;
	if(_a->b > _b->b)
		return true;
	if(_a->c > _b->c)
		return true;
	if(_a->d > _b->d)
		return true;
	if(_a->e1 > _b->e1)
		return true;
	if(_a->e2 > _b->e2)
		return true;
	return false;
}

static inline bool uuid_lt(const uuid_t* _a, const uuid_t* _b) {
	if(_a->a < _b->a)
		return true;
	if(_a->b < _b->b)
		return true;
	if(_a->c < _b->c)
		return true;
	if(_a->d < _b->d)
		return true;
	if(_a->e1 < _b->e1)
		return true;
	if(_a->e2 < _b->e2)
		return true;
	return false;
}

#define uuid_gteq(_a, _b) (uuid_eq(_a, _b) || uuid_gt(_a, _b))
#define uuid_lteq(_a, _b) (uuid_eq(_a, _b) || uuid_lt(_a, _b))

#endif
