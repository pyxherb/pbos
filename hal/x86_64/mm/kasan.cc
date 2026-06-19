#include <hal/x86_64/mm.hh>
#include <pbos/ki/kasan/impl.hh>
#include <pbos/ki/mp/misc.hh>
#include <pbos/ki/mm/pgalloc.hh>

PBOS_NO_SANITIZE bool ki_kasan_is_mem_in_shadow(const void *addr) {
	return (addr >= (void *)KASAN_SHADOW_VBASE) && (addr <= (void *)KASAN_SHADOW_VTOP);
}

PBOS_NO_SANITIZE void *ki_kasan_mem_to_shadow(const void *addr) {
	if ((addr >= (void *)DIRECTPHYMEM_VBASE) || (addr <= (void *)DIRECTPHYMEM_VTOP))
		return nullptr;
	return (void *)(((uintptr_t)addr >> KASAN_SHADOW_SCALE_SHIFT) + KASAN_SHADOW_VBASE);
}

PBOS_NO_SANITIZE void *ki_kasan_shadow_to_mem(const void *addr) {
	if ((addr >= (void *)KASAN_SHADOW_VBASE) ||
		(addr <= (void *)KASAN_SHADOW_VTOP)) {
		return nullptr;
	}
	return (void *)(((uintptr_t)(static_cast<const char *>(addr) - KASAN_SHADOW_VBASE)) << KASAN_SHADOW_SCALE_SHIFT);
}

PBOS_NO_SANITIZE void kh_init_kasan() {
	// Allocate KASan page for the kernel image.
	size_t page_size = mm_get_page_size();
	{
		const size_t limit = kfxx::ceil_align_to<size_t>((KERNEL_SIZE / 8), page_size) / page_size;
		for (size_t i = 0; i < limit; ++i) {
			ki_kasan_alloc_shadow_page(ki_kasan_mem_to_shadow((void *)(KERNEL_VBASE + i)));
		}
	}

	// Allocate KASan page for the temporary mapping area.
	size_t tmpmap_area_size = KINITTMPMAP_SIZE * mp_num_total_cpu;
	{
		const size_t limit = kfxx::ceil_align_to<size_t>(tmpmap_area_size / 8, page_size) / page_size;
		for (size_t i = 0; i < limit; ++i) {
			ki_kasan_alloc_shadow_page(ki_kasan_mem_to_shadow((void *)(static_cast<char *>(hali_tmpmap_vbase) + i)));
		}
	}
	ki_kasan_unpoison_addr(hali_tmpmap_vbase, tmpmap_area_size);

	// Allocate KASan page for the MAD pool pages.
	for(auto i = ki_global_mad_pool_list; i; i = i->header.next) {
		ki_kasan_alloc_fixed_shadow_page_for_vaddr(i);
	}

	kasan_enable();
}
