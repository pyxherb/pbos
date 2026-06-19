#include <pbos/ki/kf/misc.h>
#include <kernel/generated/config.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/kasan/impl.hh>

PBOS_EXTERN_C_BEGIN

bool ki_kasan_enabled = false;

PBOS_NO_SANITIZE km_result_t ki_kasan_alloc_shadow_page(const void *addr) {
	kd_dbgcheck(ki_kasan_shadow_to_mem(addr), "Address requested for allocation is not in the shadow area");

	void *paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);

	if (!paddr)
		return KM_RESULT_NO_MEM;

	kfxx::deferred free_paddr_guard([paddr]() noexcept {
		mm_unpin_page(paddr);
	});

	KM_RETURN_IF_FAILED(mm_mmap(mm_get_cur_context(), const_cast<void *>(addr), paddr, mm_get_page_size(), MM_PAGE_READ | MM_PAGE_WRITE, MM_MMAP_NO_REMAP));

	// Unpoison the shadow page.
	ki_raw_memset(const_cast<void *>(addr), 0, mm_get_page_size());

	return KM_RESULT_OK;
}

PBOS_NO_SANITIZE km_result_t ki_kasan_free_shadow_page(const void *addr) {
	kd_dbgcheck(ki_kasan_shadow_to_mem(addr), "Address requested for freeing is not in the shadow area");

	mm_munmap(mm_get_cur_context(), const_cast<void *>(addr), mm_get_page_size(), 0);

	return KM_RESULT_OK;
}

PBOS_NO_SANITIZE km_result_t ki_kasan_alloc_fixed_shadow_page_for_vaddr(void *vaddr) {
	void *shadow_page = ki_kasan_mem_to_shadow(vaddr);
	if (mm_getmap(mm_get_cur_context(), shadow_page, nullptr))
		return KM_RESULT_OK;

	void *paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);

	if (!paddr)
		return KM_RESULT_NO_MEM;

	kfxx::deferred free_paddr_guard([paddr]() noexcept {
		mm_unpin_page(paddr);
	});

	KM_RETURN_IF_FAILED(mm_mmap(mm_get_cur_context(), shadow_page, paddr, mm_get_page_size(), MM_PAGE_READ | MM_PAGE_WRITE, MM_MMAP_NO_REMAP));

	// Unpoison the shadow page.
	ki_raw_memset(shadow_page, 0, mm_get_page_size());

	return KM_RESULT_OK;
}

PBOS_NO_SANITIZE PBOS_API bool kasan_is_available() {
#if !KI_ENABLE_KASAN
	return false;
#else
	return ki_kasan_enabled;
#endif
}

PBOS_NO_SANITIZE PBOS_API void kasan_enable() {
	ki_kasan_enabled = true;
}

PBOS_NO_SANITIZE PBOS_API void kasan_disable() {
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
