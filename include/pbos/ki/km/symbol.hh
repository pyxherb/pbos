#ifndef _PBOS_KI_KM_SYMBOL_HH_
#define _PBOS_KI_KM_SYMBOL_HH_

#include <pbos/common.h>
#include <cstddef>
#include <type_traits>

PBOS_EXTERN_C_BEGIN

typedef struct _ki_syment_t {
	const char *name;
	size_t len_name;
	void *addr;
} ki_syment_t;

static_assert(std::is_trivial_v<ki_syment_t>, "You must make ki_syment_t trivial to support kernel image symbol exporting");

extern const ki_syment_t KI_EXPORTED_SYMBOLS_BEGIN[], KI_EXPORTED_SYMBOLS_END[];

#define KI_EXPORT_IMAGE_SYMBOL(s)                                                                     \
	const char _sym_name##s[] = #s;                                                                   \
	PBOS_USED PBOS_IN_SECTION(".exported_image_sym") volatile const ki_syment_t _exported_sym_##s = { \
		.name = _sym_name##s,                                                                         \
		.len_name = sizeof(_sym_name##s),                                                             \
		.addr = (void *)s                                                                             \
	}

PBOS_EXTERN_C_END

#endif
