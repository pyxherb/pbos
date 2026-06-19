#include <pbos/ki/kf/misc.h>
#include <kernel/generated/config.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/kh/mm/misc.hh>
#include <pbos/ki/kasan/impl.hh>

PBOS_EXTERN_C_BEGIN

bool ki_kasan_enabled = false;

PBOS_NO_ASAN void ki_kasan_scan_and_recycle_shadow_pages(void *vaddr, size_t size) {
	uintptr_t floor_vaddr = kfxx::floor_align_to<uintptr_t>((uintptr_t)vaddr, mm_get_page_size() * 8),
			  ceil_top = kfxx::floor_align_to<uintptr_t>((uintptr_t)(static_cast<char *>(vaddr) + size), mm_get_page_size() * 8);

	const size_t delta = mm_get_page_size() * 8;

	for (size_t i = 0; i < size; i += delta) {
		bool recyclable = true;
		kh_walk_pgtab(mm_get_cur_context(), reinterpret_cast<void *>(floor_vaddr + i), size, [](void *vaddr, void *paddr, mm_page_access_t page_access, void *user_data) -> kf_control_flow_t {
			*static_cast<bool *>(user_data) = true;
			return KF_CONTROL_FLOW_BREAK; }, &recyclable, KH_WALK_PGTAB_SKIP_UNMAPPED);
		if (recyclable) {
			mm_munmap(mm_get_cur_context(), ki_kasan_mem_to_shadow(reinterpret_cast<void *>(floor_vaddr + i)), PAGESIZE, MM_MMAP_IGNORE_KASAN);
		}
	}
}

PBOS_NO_ASAN km_result_t ki_kasan_alloc_shadow_pages_for_vaddr(void *vaddr, size_t size) {
	mm_context_t *const context = mm_get_cur_context();

	void *shadow_pages_begin = ki_kasan_mem_to_shadow(vaddr), *shadow_pages_end = ki_kasan_mem_to_shadow(static_cast<char *>(vaddr) + size);
	if(!shadow_pages_begin)
		return KM_RESULT_OK;
	const size_t delta = mm_get_page_size();

	kfxx::scope_guard unmap_guard([context, vaddr, size, shadow_pages_begin, shadow_pages_end]() noexcept {
		kh_munmap(context, shadow_pages_begin, static_cast<char *>(shadow_pages_end) - static_cast<char *>(shadow_pages_begin), MM_MUNMAP_IGNORE_KASAN);
	});

	const size_t limit = size / 8;

	for (size_t i = 0; i < limit; i += delta) {
		void *cur_page = static_cast<char *>(shadow_pages_begin) + i;
		if (mm_getmap(mm_get_cur_context(), cur_page, nullptr))
			return KM_RESULT_OK;

		void *paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);

		if (!paddr)
			return KM_RESULT_NO_MEM;

		kfxx::deferred free_paddr_guard([paddr]() noexcept {
			mm_unpin_page(paddr);
		});

		KM_RETURN_IF_FAILED(kh_mmap(mm_get_cur_context(), cur_page, paddr, mm_get_page_size(), MM_PAGE_READ | MM_PAGE_WRITE, MM_MMAP_NO_REMAP));

		// Unpoison the shadow page.
		ki_raw_memset(cur_page, 0, mm_get_page_size());
	}

	unmap_guard.release();

	return KM_RESULT_OK;
}

PBOS_NO_ASAN PBOS_API bool kasan_is_available() {
#if !KI_ENABLE_KASAN
	return false;
#else
	return ki_kasan_enabled;
#endif
}

PBOS_NO_ASAN PBOS_API void kasan_enable() {
	ki_kasan_enabled = true;
}

PBOS_NO_ASAN PBOS_API void kasan_disable() {
	ki_kasan_enabled = false;
}

PBOS_NO_ASAN void ki_kasan_report(const void *addr, size_t size, bool is_write, void *return_ip) {
	// TODO: Use %zu instead after we implemented it.
	km_panic("KASan %s error on address %p, size=%u, return_ip = %p", is_write ? "write" : "read", addr, (unsigned int)size, return_ip);
}

PBOS_FORCEINLINE bool _verify_area(void *addr, size_t size, bool is_write, void *return_ip) {
	if (!kasan_is_available())
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

PBOS_NO_ASAN void ki_kasan_poison_addr(void *addr, size_t size, uint8_t value) {
	if (kasan_is_available()) {
		if (reinterpret_cast<uintptr_t>(addr) & (KASAN_GRANULE_SIZE - 1))
			km_panic("Unaligned address poisoning on %p", addr);
		if (size & (KASAN_GRANULE_SIZE - 1))
			// TODO: Use %zu.
			km_panic("Address poisoning on %p with unaligned size %u", addr, (unsigned int)size);
		char *start = static_cast<char *>(ki_kasan_mem_to_shadow(addr)), *end = static_cast<char *>(ki_kasan_mem_to_shadow(static_cast<char *>(addr) + size));

		ki_raw_memset(start, value, end - start);
	}
}

PBOS_NO_ASAN void ki_kasan_unpoison_addr(void *addr, size_t size) {
	if (reinterpret_cast<uintptr_t>(addr) & KASAN_GRANULE_SIZE)
		km_panic("Unaligned address poisoning on %p", addr);
	ki_kasan_poison_addr(addr, kfxx::ceil_align_to<size_t, KASAN_GRANULE_SIZE>(size), 0);
}

PBOS_NO_ASAN void ki_kasan_poison_last_granule(void *addr, size_t size) {
	if (kasan_is_available()) {
		if (size & KASAN_GRANULE_SIZE) {
			char *shadow = static_cast<char *>(ki_kasan_mem_to_shadow(static_cast<char *>(addr) + size));
			*shadow = size & (KASAN_GRANULE_SIZE - 1);
		}
	}
}

PBOS_EXTERN_C_END
