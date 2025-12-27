#include <pbos/hal/irq.hh>
#include "../mm.h"
#include <pbos/km/proc.h>

PBOS_EXTERN_C_BEGIN

arch_pde_t *const hn_kernel_pdt = ((arch_pde_t *)KPDT_VBASE);
arch_pte_t *const hn_kernel_pgt = ((arch_pte_t *)KPGT_VBASE);
arch_pte_t *const hn_bottom_pgt = ((arch_pte_t *)KBOTTOMPGT_VBASE);

mm_context_t *mm_get_cur_context() {
	return mm_cur_contexts ? mm_cur_contexts[ps_get_cur_euid()] : mm_kernel_context;
}

void mm_invlpg(void *ptr) {
	arch_invlpg(ptr);
}

bool mm_is_user_space(const void *ptr) {
	if (((ptr >= (char *)KERNEL_VBASE) ||
		(ptr <= (char *)KBOTTOM_VTOP)))
		return false;

	return true;
}

bool mm_probe_user_space(mm_context_t *mm_context, const void *ptr, size_t size) {
	io::irq_disable_lock irq_lock;

	const char *p = (const char *)PGFLOOR((uintptr_t)ptr), *limit = p + PGCEIL(size);
	mm_pgaccess_t pgaccess;

	// Overflow is an error.
	if (UINTPTR_MAX - (uintptr_t)p < PGCEIL(size))
		return true;

	if((!mm_is_user_space(ptr)) || (!mm_is_user_space(limit)))
		return true;

	for (size_t i = 0; i < PGCEIL(size); ++i) {
		mm_getmap(mm_context, p, &pgaccess);
		if ((!(pgaccess & PAGE_MAPPED)) || (!(pgaccess & PAGE_USER)))
			return true;
	}

	return false;
}

uint8_t hn_to_kn_pmem_type(uint8_t memtype) {
	switch (memtype) {
		case MM_PMEM_AVAILABLE:
			return KN_PMEM_AVAILABLE;
		case MM_PMEM_HARDWARE:
			return KN_PMEM_HARDWARE;
		case MM_PMEM_HIBERNATION:
			return KN_PMEM_HIBERNATION;
		case MM_PMEM_ACPI:
			return KN_PMEM_ACPI;
	}
	return KN_PMEM_END;
}

PBOS_EXTERN_C_END
