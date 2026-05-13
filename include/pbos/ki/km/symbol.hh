#ifndef _PBOS_KI_KM_SYMBOL_HH_
#define _PBOS_KI_KM_SYMBOL_HH_

#include <pbos/common.h>
#include <cstddef>
#include <type_traits>
#include <elf.h>

PBOS_EXTERN_C_BEGIN

typedef struct _ki_syment_t {
	const char *name;
	size_t len_name;
	void *addr;
} ki_syment_t;

static_assert(std::is_trivial_v<ki_syment_t>, "You must make ki_syment_t trivial to support kernel image symbol exporting");

extern const Elf64_Sym KI_EXPORTED_SYMBOLS_BEGIN[], KI_EXPORTED_SYMBOLS_END[];

PBOS_EXTERN_C_END

#endif
