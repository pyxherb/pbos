#include "../mm.h"
#include "../proc.h"

mm_context_t hn_kernel_mmctxt;
mm_context_t *mm_kernel_context = &hn_kernel_mmctxt;
mm_context_t **mm_current_contexts;

void mm_copy_global_mappings(mm_context_t *dest, const mm_context_t *src) {
	memcpy(
		dest->pdt + PDX(KBOTTOM_VBASE),
		src->pdt + PDX(KBOTTOM_VBASE),
		sizeof(arch_pde_t) * PDX(KBOTTOM_SIZE));
	memcpy(
		dest->pdt + PDX(KSPACE_VBASE),
		src->pdt + PDX(KSPACE_VBASE),
		sizeof(arch_pde_t) * PDX(KSPACE_SIZE));
}

void mm_sync_global_mappings(const mm_context_t *src) {
	for (ps_euid_t i = 0; i < ps_eu_num; ++i) {
		mm_context_t *cur_context = mm_current_contexts[i];

		if (cur_context == src)
			continue;

		mm_copy_global_mappings(cur_context, src);
	}
}

km_result_t mm_create_context(mm_context_t *context) {
	km_result_t result;
	void *pdt_paddr = NULL,
		 *pdt_vaddr = NULL;
	if (!(pdt_paddr = mm_pgalloc(MM_PMEM_AVAILABLE))) {
		result = KM_RESULT_NO_MEM;
		goto fail;
	}
	if (!(pdt_vaddr = mm_kvmalloc(mm_kernel_context, PAGESIZE, PAGE_READ | PAGE_WRITE, 0))) {
		result = KM_RESULT_NO_MEM;
		goto fail;
	}

	for (size_t i = 0; i < PB_ARRAYSIZE(context->uspace_vpm_query_tree); ++i) {
		kf_rbtree_init(
			&context->uspace_vpm_query_tree[i],
			hn_vpm_nodecmp,
			hn_vpm_nodefree);
	}
	if (KM_FAILED(result = mm_mmap(mm_kernel_context, pdt_vaddr, pdt_paddr, PAGESIZE, PAGE_READ | PAGE_WRITE, 0))) {
		goto fail;
	}
	context->pdt = pdt_vaddr;

	mm_copy_global_mappings(context, mm_kernel_context);

	// mm_copy_global_mappings(context, mm_kernel_context);
	return KM_RESULT_OK;

fail:
	if (pdt_paddr) {
		mm_pgfree(pdt_paddr);
	}
	if (pdt_vaddr) {
		mm_vmfree(mm_kernel_context, pdt_vaddr, PAGESIZE);
	}
}

void mm_free_context(mm_context_t *context) {
	for (uint16_t i = 0; i < PDX_MAX; ++i) {
		arch_pde_t *pde = &(context->pdt[i]);
		if (pde->mask & PDE_P)
			mm_pgfree(UNPGADDR(pde->address));
	}

	// Free VPD query tree and pools.
	for (size_t i = 0; i < PB_ARRAYSIZE(context->uspace_vpm_query_tree); ++i) {
		kf_rbtree_free(&context->uspace_vpm_query_tree[i]);
	}
	for (hn_vpm_poolpg_t *i = context->uspace_vpm_poolpg_list; i; i = i->header.next) {
		void *paddr = mm_getmap(mm_kernel_context, i);
		mm_unmmap(mm_kernel_context, i, PAGESIZE, 0);
		mm_pgfree(paddr);
	}

	mm_pgfree(context->pdt);
}

void mm_switch_context(mm_context_t *context) {
	mm_context_t *prev_context = mm_current_contexts[ps_get_current_euid()];
	mm_current_contexts[ps_get_current_euid()] = context;
	mm_sync_global_mappings(prev_context);
	arch_lpdt(PGROUNDDOWN(hn_getmap(mm_kernel_context->pdt, context->pdt)));
}
