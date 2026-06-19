#include <hal/x86_64/mm.hh>
#include <pbos/ki/kasan/impl.hh>
#include <pbos/ki/mm/pgalloc.hh>
#include <pbos/ki/mp/misc.hh>

PBOS_EXTERN_C_BEGIN

PBOS_NO_ASAN bool kh_kasan_is_mem_in_shadow(const void *addr) {
	return (addr >= (void *)KASAN_SHADOW_VBASE) && (addr <= (void *)KASAN_SHADOW_VTOP);
}

PBOS_NO_ASAN void *kh_kasan_mem_to_shadow(const void *addr) {
	if ((addr < (void *)KSPACE_VBASE))
		return nullptr;
	if ((addr >= (void *)DIRECTPHYMEM_VBASE) && (addr <= (void *)DIRECTPHYMEM_VTOP))
		return nullptr;
	return (void *)(((uintptr_t)(static_cast<const char*>(addr) - KSPACE_VBASE) >> 3) + KASAN_SHADOW_VBASE);
}

PBOS_NO_ASAN void *kh_kasan_shadow_to_mem(const void *addr) {
	if ((addr >= (void *)KASAN_SHADOW_VBASE) &&
		(addr <= (void *)KASAN_SHADOW_VTOP)) {
		return (void *)(((uintptr_t)(static_cast<const char *>(addr) - KASAN_SHADOW_VBASE)) * 8);
	}
	return nullptr;
}

PBOS_NO_ASAN void kh_init_kasan() {
	// Allocate KASan page for the kernel image.
	size_t page_size = mm_get_page_size();
	ki_kasan_alloc_shadow_pages_for_vaddr((void *)KERNEL_VBASE, KERNEL_SIZE);

	// Allocate KASan page for the temporary mapping area.
	size_t tmpmap_area_size = KINITTMPMAP_SIZE * mp_num_total_cpu;
	ki_kasan_alloc_shadow_pages_for_vaddr(hali_tmpmap_vbase, tmpmap_area_size);

	// Allocate KASan page for the MAD pool pages.
	for (auto i = ki_global_mad_pool_list; i; i = i->header.next) {
		ki_kasan_alloc_shadow_pages_for_vaddr(i, page_size);
	}

	kasan_enable();
}

PBOS_EXTERN_C_END
