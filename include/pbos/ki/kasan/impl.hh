#ifndef _PBOS_KASAN_IMPL_HH_
#define _PBOS_KASAN_IMPL_HH_

#include <pbos/generated/mm.h>
#include <pbos/kasan/handlers.h>
#include <stddef.h>
#include KI_ARCH_MEMCONF_HEADER_PATH

PBOS_EXTERN_C_BEGIN

PBOS_NO_SANITIZE bool ki_kasan_is_available();

PBOS_NO_SANITIZE void ki_kasan_enable();
PBOS_NO_SANITIZE void ki_kasan_disable();

PBOS_NO_SANITIZE void ki_kasan_report(const void *addr, size_t size, bool is_write, void *return_ip);;

PBOS_NO_SANITIZE PBOS_FORCEINLINE bool ki_kasan_is_mem_in_shadow(const void *addr) {
	return (addr >= (void *)KASAN_SHADOW_VBASE) && (addr <= (void *)KASAN_SHADOW_VTOP);
}

PBOS_NO_SANITIZE PBOS_FORCEINLINE void *ki_kasan_mem_to_shadow(const void *addr) {
	return (void *)(((uintptr_t)addr >> KASAN_SHADOW_SCALE_SHIFT) + KASAN_SHADOW_VBASE);
}

PBOS_NO_SANITIZE PBOS_FORCEINLINE void *ki_kasan_shadow_to_mem(const void *addr) {
	if ((addr >= (void *)KASAN_SHADOW_VBASE) ||
		(addr <= (void *)KASAN_SHADOW_VTOP)) {
		return nullptr;
	}
	return (void *)(((uintptr_t)(static_cast<const char *>(addr) - KASAN_SHADOW_VBASE)) << KASAN_SHADOW_SCALE_SHIFT);
}

PBOS_NO_SANITIZE PBOS_FORCEINLINE bool ki_kasan_is_byte_poisoned(const void *addr) {
	const uint8_t shadow = *static_cast<const uint8_t *>(ki_kasan_mem_to_shadow(addr));

	if (!shadow)
		return false;

	uint8_t last_byte = ((uintptr_t)addr) & (KASAN_GRANULE_SIZE - 1);
	return last_byte >= shadow;
}

PBOS_NO_SANITIZE PBOS_FORCEINLINE bool ki_kasan_is_area_poisoned(const void *addr, size_t size) {
	char *start = static_cast<char *>(ki_kasan_mem_to_shadow(addr)),
		 *end = static_cast<char *>(ki_kasan_mem_to_shadow(static_cast<const char *>(addr) + size));

	kfxx::option_t<size_t> nonzero_off;

	for (size_t i = 0; i < size; ++i) {
		if (start[i]) {
			nonzero_off = +i;
			break;
		}
	}

	if (nonzero_off.has_value()) {
		const char *last_byte = static_cast<const char *>(addr) + size - 1;
		uint8_t *last_shadow = (uint8_t *)ki_kasan_mem_to_shadow(last_byte);
		uint8_t last_accessible_byte = ((uintptr_t)last_byte) & (KASAN_GRANULE_SIZE - 1);

		if (nonzero_off.value() != (size_t)last_shadow ||
			last_accessible_byte >= *last_shadow)
			return true;
	}

	return false;
}

PBOS_NO_SANITIZE bool ki_kasan_is_available();

PBOS_NO_SANITIZE void ki_kasan_poison_addr(void *addr, size_t size, uint8_t value);
PBOS_NO_SANITIZE void ki_kasan_unpoison_addr(void *addr, size_t size);

PBOS_NO_SANITIZE void ki_kasan_poison_last_granule(void *addr, size_t size);

PBOS_EXTERN_C_END

#endif
