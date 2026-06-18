#include <pbos/ki/kf/misc.h>
#include <kernel/generated/config.hh>
#include <pbos/ki/kasan/impl.hh>

PBOS_EXTERN_C_BEGIN

bool ki_kasan_enabled = false;

PBOS_NO_SANITIZE bool ki_kasan_is_available() {
#if !KI_ENABLE_KASAN
	return false;
#else
	return ki_kasan_enabled;
#endif
}

PBOS_NO_SANITIZE void ki_kasan_enable() {
	ki_kasan_enabled = true;
}

PBOS_NO_SANITIZE void ki_kasan_disable() {
	ki_kasan_enabled = false;
}

PBOS_NO_SANITIZE void ki_kasan_report(const void *addr, size_t size, bool is_write, void *return_ip) {
	// TODO: Use %zu instead after we implemented it.
	km_panic("KASan %s error on address %p, size=%u, return_ip = %p", is_write ? "write" : "read", addr, (unsigned int)size, return_ip);
}

PBOS_FORCEINLINE bool _verify_area(void *addr, size_t size, bool is_write, void *return_ip) {
	if (!ki_kasan_is_available())
		return true;

	if (!size)
		return true;

	if ((static_cast<char *>(addr) + size < static_cast<char *>(addr)))
		ki_kasan_report(addr, size, is_write, return_ip);

	if (!ki_kasan_mem_to_shadow(addr))
		ki_kasan_report(addr, size, is_write, return_ip);

	if (ki_kasan_is_area_poisoned(addr, size))
		ki_kasan_report(addr, size, is_write, return_ip);

	return false;
}

PBOS_NO_SANITIZE void ki_kasan_poison_addr(void *addr, size_t size, uint8_t value) {
	if (ki_kasan_is_available()) {
		if (reinterpret_cast<uintptr_t>(addr) & (KASAN_GRANULE_SIZE - 1))
			km_panic("Unaligned address poisoning on %p", addr);
		if (size & (KASAN_GRANULE_SIZE - 1))
			// TODO: Use %zu.
			km_panic("Address poisoning on %p with unaligned size %u", addr, (unsigned int)size);
		char *start = static_cast<char *>(ki_kasan_mem_to_shadow(addr)), *end = static_cast<char *>(ki_kasan_mem_to_shadow(static_cast<char *>(addr) + size));

		// TODO: memset it.
		ki_raw_memset(start, value, end - start);
	}
}

PBOS_NO_SANITIZE void ki_kasan_unpoison_addr(void *addr, size_t size) {
	if (reinterpret_cast<uintptr_t>(addr) & KASAN_GRANULE_SIZE)
		km_panic("Unaligned address poisoning on %p", addr);
	ki_kasan_poison_addr(addr, kfxx::ceil_align_to<size_t, KASAN_GRANULE_SIZE>(size), 0);
}

PBOS_NO_SANITIZE void ki_kasan_poison_last_granule(void *addr, size_t size) {
	if (ki_kasan_is_available()) {
		if (size & KASAN_GRANULE_SIZE) {
			char *shadow = static_cast<char *>(ki_kasan_mem_to_shadow(static_cast<char *>(addr) + size));
			*shadow = size & (KASAN_GRANULE_SIZE - 1);
		}
	}
}

PBOS_EXTERN_C_END
