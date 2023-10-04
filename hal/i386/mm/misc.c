#include "../mm.h"

mm_context_t hn_kernel_mmctxt;
mm_context_t *mm_kernel_context = &hn_kernel_mmctxt;

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

void mm_copy_kspace_pgtab(mm_context_t *context) {
	memcpy(
		context->pdt + PDX(KBOTTOM_VBASE),
		mm_kernel_context->pdt + PDX(KBOTTOM_VBASE),
		sizeof(arch_pde_t) * PDX(KBOTTOM_SIZE));
	memcpy(
		context->pdt + PDX(KSPACE_VBASE),
		mm_kernel_context->pdt + PDX(KSPACE_VBASE),
		sizeof(arch_pde_t) * PDX(KSPACE_SIZE));
}

__nodiscard mm_context_t *mm_create_context() {
	mm_context_t *context = mm_kmalloc(sizeof(mm_context_t));
	if (!context)
		return NULL;

	void *paddr = mm_pgalloc(MM_PMEM_AVAILABLE, PAGE_READ | PAGE_WRITE, 0),
		 *vaddr = UNPGADDR(hn_vpgalloc(hn_kernel_mmctxt.pdt, PGROUNDDOWN(KSPACE_VBASE), PGADDR_MAX));
	context->pdt = vaddr;
	mm_mmap(&hn_kernel_mmctxt, vaddr, paddr, PAGESIZE, PAGE_READ | PAGE_WRITE);

	mm_copy_kspace_pgtab(context);
	return context;
}

void mm_free_context(mm_context_t *context) {
	for (uint16_t i = 0; i < PDX_MAX; ++i) {
		arch_pde_t *pde = &(context->pdt[i]);
		if (pde->mask & PDE_P)
			mm_pgfree(UNPGADDR(pde->address), 0);
	}
	mm_pgfree(context->pdt, 0);
}

void mm_switch_context(mm_context_t *context) {
	mm_copy_kspace_pgtab(context);
	arch_lpdt(PGROUNDDOWN(hn_getmap(mm_kernel_context->pdt, context->pdt)));
}
