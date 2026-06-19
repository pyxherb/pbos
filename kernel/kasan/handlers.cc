#include <pbos/ki/kf/misc.h>
#include <pbos/ki/kasan/impl.hh>

PBOS_EXTERN_C_BEGIN

PBOS_NO_ASAN PBOS_API void __asan_register_globals(void *globals, size_t size) {
	kasan_global_t *ptr = static_cast<kasan_global_t *>(globals);

	for (size_t i = 0; i < size; ++i) {
		size_t aligned_size = kfxx::ceil_align_to<size_t, KASAN_GRANULE_SIZE>(ptr[i].size);

		ki_kasan_unpoison_addr(ptr[i].begin, ptr[i].size);

		ki_kasan_poison_addr(static_cast<char *>(ptr[i].begin) + aligned_size, ptr[i].size_with_redzone - aligned_size, KASAN_SHADOW_VALUE_GLOBAL_REDZONE);
	}
}

PBOS_NO_ASAN PBOS_API void __asan_unregister_globals(void *globals, size_t size) {
	// Do nothing.
}

PBOS_NO_ASAN PBOS_API void __asan_before_dynamic_init(const char *module_name) {
}

PBOS_NO_ASAN PBOS_API void __asan_after_dynamic_init(void) {
}

PBOS_NO_ASAN PBOS_API void __asan_handle_no_return(void) {
	// To prevent the compiler to complain.
}

PBOS_NO_ASAN PBOS_API void __asan_alloca_poison(void *addr, size_t size) {
	if (!kasan_is_available())
		return;

	size_t ceil_size = kfxx::ceil_align_to<size_t, KASAN_GRANULE_SIZE>(size);
	size_t padding_size = kfxx::ceil_align_to<size_t, KASAN_ALLOCA_REDZONE_SIZE>(size) -
						  ceil_size;
	size_t floor_size = kfxx::floor_align_to<size_t, KASAN_GRANULE_SIZE>(size);

	if (reinterpret_cast<uintptr_t>(addr) & (KASAN_ALLOCA_REDZONE_SIZE - 1))
		km_panic("Poisoning alloca unaligned: %p", addr);

	ki_kasan_unpoison_addr(static_cast<char *>(addr) + floor_size, size - floor_size);

	void *redzone_l = static_cast<void *>(static_cast<char *>(addr) - KASAN_ALLOCA_REDZONE_SIZE);
	void *redzone_r = static_cast<void *>(static_cast<char *>(addr) + ceil_size);

	ki_kasan_poison_addr(redzone_l, KASAN_ALLOCA_REDZONE_SIZE, KASAN_ALLOCA_LEFT);
	ki_kasan_poison_addr(redzone_r, padding_size + KASAN_ALLOCA_REDZONE_SIZE, KASAN_ALLOCA_RIGHT);
}
PBOS_NO_ASAN PBOS_API void __asan_allocas_unpoison(void *stack_top, void *stack_bottom) {
	if (!kasan_is_available())
		return;
	if ((!stack_top) || (stack_top > stack_bottom))
		return;

	ki_kasan_unpoison_addr(stack_top, static_cast<char *>(stack_bottom) - static_cast<char *>(stack_top));
}

PBOS_NO_ASAN PBOS_API void __asan_load1(void *addr) {
	if (ki_kasan_is_area_poisoned(addr, 1))
		ki_kasan_report(addr, 1, false, __builtin_return_address(0));
}

PBOS_NO_ASAN PBOS_API void __asan_store1(void *addr) {
	if (ki_kasan_is_area_poisoned(addr, 1))
		ki_kasan_report(addr, 1, true, __builtin_return_address(0));
}

#define KASAN_LOAD_STORE_FN(width)                                            \
	PBOS_NO_ASAN PBOS_API void __asan_load##width(void *addr) {               \
		if (ki_kasan_is_area_poisoned(addr, width))                           \
			ki_kasan_report(addr, width, false, __builtin_return_address(0)); \
	}                                                                         \
	PBOS_NO_ASAN PBOS_API void __asan_store##width(void *addr) {              \
		if (ki_kasan_is_area_poisoned(addr, width))                           \
			ki_kasan_report(addr, width, true, __builtin_return_address(0));  \
	}

KASAN_LOAD_STORE_FN(2);

KASAN_LOAD_STORE_FN(4);

KASAN_LOAD_STORE_FN(8);

KASAN_LOAD_STORE_FN(16);

PBOS_NO_ASAN PBOS_API void __asan_loadN(void *addr, size_t size) {
	if (ki_kasan_is_area_poisoned(addr, size))
		ki_kasan_report(addr, size, false, __builtin_return_address(0));
}
PBOS_NO_ASAN PBOS_API void __asan_storeN(void *addr, size_t size) {
	if (ki_kasan_is_area_poisoned(addr, size))
		ki_kasan_report(addr, 1, true, __builtin_return_address(0));
}

#define KASAN_LOAD_STORE_NOABORT_FN(width)                                 \
	PBOS_NO_ASAN PBOS_API void __asan_load##width##_noabort(void *addr) {  \
		__asan_load##width(addr);                                          \
	}                                                                      \
	PBOS_NO_ASAN PBOS_API void __asan_store##width##_noabort(void *addr) { \
		__asan_store##width(addr);                                         \
	}

KASAN_LOAD_STORE_NOABORT_FN(1);

KASAN_LOAD_STORE_NOABORT_FN(2);

KASAN_LOAD_STORE_NOABORT_FN(4);

KASAN_LOAD_STORE_NOABORT_FN(8);

KASAN_LOAD_STORE_NOABORT_FN(16);

PBOS_NO_ASAN PBOS_API void __asan_loadN_noabort(void *addr, size_t size) {
	__asan_loadN(addr, size);
}
PBOS_NO_ASAN PBOS_API void __asan_storeN_noabort(void *addr, size_t size) {
	__asan_storeN(addr, size);
}

#define KASAN_REPORT_LOAD_STORE_FN(width)                                         \
	PBOS_NO_ASAN PBOS_API void __asan_report_load##width##_noabort(void *addr) {  \
		ki_kasan_report(addr, width, false, __builtin_return_address(0));         \
	}                                                                             \
	PBOS_NO_ASAN PBOS_API void __asan_report_store##width##_noabort(void *addr) { \
		ki_kasan_report(addr, width, true, __builtin_return_address(0));          \
	}

KASAN_REPORT_LOAD_STORE_FN(1);

KASAN_REPORT_LOAD_STORE_FN(2);

KASAN_REPORT_LOAD_STORE_FN(4);

KASAN_REPORT_LOAD_STORE_FN(8);

KASAN_REPORT_LOAD_STORE_FN(16);

PBOS_NO_ASAN PBOS_API void __asan_report_load_n_noabort(void *addr, size_t size) {
	ki_kasan_report(addr, size, false, __builtin_return_address(0));
}
PBOS_NO_ASAN PBOS_API void __asan_report_store_n_noabort(void *addr, size_t size) {
	ki_kasan_report(addr, size, true, __builtin_return_address(0));
}

#define KASAN_SET_SHADOW_FN(value)                            \
	void __asan_set_shadow_##value(void *addr, size_t size) { \
		ki_raw_memset(addr, 0x##value, size);                 \
	}

KASAN_SET_SHADOW_FN(00);

KASAN_SET_SHADOW_FN(f1);

KASAN_SET_SHADOW_FN(f2);

KASAN_SET_SHADOW_FN(f3);

KASAN_SET_SHADOW_FN(f5);

KASAN_SET_SHADOW_FN(f8);

PBOS_NO_ASAN PBOS_API void *__asan_memset(void *addr, int c, size_t len) {
	if (ki_kasan_is_area_poisoned(addr, len))
		return nullptr;
	return ki_raw_memset(addr, c, len);
}
PBOS_NO_ASAN PBOS_API void *__asan_memmove(void *dest, const void *src, size_t len) {
	if (ki_kasan_is_area_poisoned(dest, len) ||
		ki_kasan_is_area_poisoned(src, len))
		return nullptr;
	return ki_raw_memmove(dest, src, len);
}
PBOS_NO_ASAN PBOS_API void *__asan_memcpy(void *dest, const void *src, size_t len) {
	if (ki_kasan_is_area_poisoned(dest, len) ||
		ki_kasan_is_area_poisoned(src, len))
		return nullptr;
	return ki_raw_memcpy(dest, src, len);
}

PBOS_EXTERN_C_END
