#ifndef _PBOS_KASAN_IMPL_HH_
#define _PBOS_KASAN_IMPL_HH_

#include <pbos/generated/mm.h>
#include <pbos/kasan/handlers.h>
#include <pbos/kd/logger.h>
#include <stddef.h>
#include KI_ARCH_MEMCONF_HEADER_PATH

PBOS_EXTERN_C_BEGIN

PBOS_NO_ASAN void ki_kasan_report(const void *addr, size_t size, bool is_write, void *return_ip);

PBOS_NO_ASAN bool kh_kasan_is_mem_in_shadow(const void *addr);

PBOS_NO_ASAN void *kh_kasan_mem_to_shadow(const void *addr);

PBOS_NO_ASAN void *kh_kasan_shadow_to_mem(const void *addr);

PBOS_NO_ASAN void ki_kasan_scan_and_recycle_shadow_pages(void *vaddr, size_t size);

PBOS_NO_ASAN km_result_t ki_kasan_alloc_shadow_pages_for_vaddr(void *vaddr, size_t size);

/*PBOS_NO_ASAN PBOS_FORCEINLINE bool ki_kasan_is_byte_poisoned(const void *addr) {
	if (!kasan_is_supported())
		return false;
	const void *shadow_ptr = kh_kasan_mem_to_shadow(addr);
	if (!shadow_ptr)
		return false;

	const uint8_t shadow = *static_cast<const uint8_t *>(shadow_ptr);

	if (shadow) {
		uint8_t last_byte = ((uintptr_t)addr) & (KASAN_GRANULE_SIZE - 1);
		return last_byte >= shadow;
	}

	return false;
}*/

PBOS_NO_ASAN PBOS_FORCEINLINE bool ki_kasan_is_area_poisoned(const void *addr, size_t size) {
	if (!kasan_is_supported())
		return false;
	char *start = static_cast<char *>(kh_kasan_mem_to_shadow(addr)),
		 *end = static_cast<char *>(kh_kasan_mem_to_shadow(static_cast<const char *>(addr) + size - 1)) + 1;

	if (!start)
		return false;

	void *nonzero_start = nullptr;

	while (start < end) {
		if (*start) {
			nonzero_start = static_cast<void *>(start);
			break;
		}
		++start;
	}

	if (nonzero_start) {
		const char *last_byte = static_cast<const char *>(addr) + size - 1;
		uint8_t *last_shadow = (uint8_t *)kh_kasan_mem_to_shadow(last_byte);
		uint8_t last_accessible_byte = ((uintptr_t)last_byte) & (KASAN_GRANULE_SIZE - 1);

		if (nonzero_start != last_shadow)
			km_panic("Area %p-%p poisoned, reason: nonzero_start != last_shadow", addr, (char *)addr + size);

		if (last_accessible_byte < *last_shadow)
			km_panic("Area %p-%p poisoned, reason: last_accessible_byte < *last_shadow", addr, (char *)addr + size);

		if ((nonzero_start != last_shadow) ||
			// Fuck this shit, Linux may had some buggy codes that misled us to
			// use >= instead, understand what you've learned well before you leap.
			(last_accessible_byte < *last_shadow))
			return true;
	}

	return false;
}

PBOS_NO_ASAN void ki_kasan_poison_addr(void *addr, size_t size, uint8_t value);
PBOS_NO_ASAN void ki_kasan_unpoison_addr(void *addr, size_t size);

PBOS_NO_ASAN void ki_kasan_poison_last_granule(void *addr, size_t size);

PBOS_NO_ASAN void kh_init_kasan();

PBOS_EXTERN_C_END

#endif
