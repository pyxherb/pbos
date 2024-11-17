#ifndef _PBOS_KM_SYMBOL_H_
#define _PBOS_KM_SYMBOL_H_

#include <pbos/attribs.h>

typedef struct _km_syment_t {
	void *ptr;
	size_t len;
	const char name[64 - sizeof(void *)];
} km_syment_t;

#define KM_SYMEXPORT(sym)                                              \
	PB_IN_SECTION("EXPORTED_SYMBOLS") PB_USED km_syment_t __symexport_##sym = { \
		.ptr = (void *)sym,                                            \
		.len = sizeof(#sym),                                           \
		.name = #sym                                                   \
	}

#endif
