#ifndef _PBOS_KASAN_UTILS_H_
#define _PBOS_KASAN_UTILS_H_

#include <pbos/mm/mm.h>

PBOS_EXTERN_C_BEGIN

#if __GNUC__ >= 7
	#define KASAN_ABI_VERSION 5
#else
	#define KASAN_ABI_VERSION 4
#endif

enum {
	KASAN_SHADOW_SCALE_SHIFT = 3
};

enum {
	KASAN_GRANULE_SIZE = 1UL << KASAN_SHADOW_SCALE_SHIFT
};

enum {
	// TODO: Can we use any another value? Linux kernel uses this.
	KASAN_SHADOW_VALUE_GLOBAL_REDZONE = 0xf9
};

// Specified by compiler ABI.
enum {
	KASAN_SHADOW_VALUE_STACK_LEFT = 0xf1,
	KASAN_SHADOW_VALUE_STACK_MID,
	KASAN_SHADOW_VALUE_STACK_RIGHT,
	KASAN_SHADOW_VALUE_STACK_PARTIAL
};

// Specified by compiler ABI.
enum {
	KASAN_ALLOCA_REDZONE_SIZE = 32
};

// Seems to be not specified by the compiler ABI,
// but Linux uses it, we follow it.
enum {
	KASAN_ALLOCA_LEFT = 0xca,
	KASAN_ALLOCA_RIGHT
};

// Specified by compiler ABI.
typedef struct _kasan_source_location_t {
	const char *filename;
	int line;
	int column;
} kasan_source_location_t;

// Specified by compiler ABI.
typedef struct _kasan_global_t {
	void *begin;
	size_t size;
	size_t size_with_redzone;
	const char *name;
	const char *module_name;
	unsigned long has_dynamic_init;
#if KASAN_ABI_VERSION >= 4
	kasan_source_location_t *location;
#endif
#if KASAN_ABI_VERSION >= 5
	char *odr_indicator;
#endif
} kasan_global_t;

PBOS_NO_ASAN PBOS_API bool kasan_is_available();

PBOS_NO_ASAN PBOS_API void kasan_enable();
PBOS_NO_ASAN PBOS_API void kasan_disable();

PBOS_EXTERN_C_END

#endif
