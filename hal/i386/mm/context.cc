#include <pbos/kfxx/scope_guard.hh>
#include "../mm.hh"
#include "../proc.h"

PBOS_EXTERN_C_BEGIN

mm_context_t hn_kernel_mm_context;
mm_context_t *mm_kernel_context = &hn_kernel_mm_context;
mm_context_t **mm_cur_contexts;

void kn_mm_copy_global_mappings(mm_context_t *dest, const mm_context_t *src) {
	memcpy(
		dest->pdt + PDX(KBOTTOM_VBASE),
		src->pdt + PDX(KBOTTOM_VBASE),
		sizeof(arch_pde_t) * PDX(KBOTTOM_SIZE));
	memcpy(
		dest->pdt + PDX(KSPACE_VBASE),
		src->pdt + PDX(KSPACE_VBASE),
		sizeof(arch_pde_t) * PDX(KSPACE_SIZE));
}

void kn_mm_sync_global_mappings(const mm_context_t *src) {
	for (ps_euid_t i = 0; i < ps_eu_num; ++i) {
		mm_context_t *cur_context = mm_cur_contexts[i];

		if (cur_context == src)
			continue;

		kn_mm_copy_global_mappings(cur_context, src);
	}
}

km_result_t kn_mm_init_context(mm_context_t *context) {
	km_result_t result;

	void *pdt_paddr = NULL,
		 *pdt_vaddr = NULL;

	if (!(pdt_paddr = mm_pgalloc(MM_PMEM_AVAILABLE))) {
		return KM_RESULT_NO_MEM;
	}
	kfxx::scope_guard free_pdt_paddr_guard([pdt_paddr]() noexcept {
		mm_pgfree(pdt_paddr);
	});

	if (!(pdt_vaddr = mm_kvmalloc(mm_kernel_context, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, 0))) {
		return KM_RESULT_NO_MEM;
	}
	kfxx::scope_guard free_pdt_vaddr_guard([pdt_vaddr]() noexcept {
		mm_vmfree(mm_kernel_context, pdt_vaddr, PAGESIZE);
	});

	for (size_t i = 0; i < PBOS_ARRAYSIZE(context->uspace_vpm_query_tree); ++i) {
		kf_rbtree_init(
			&context->uspace_vpm_query_tree[i],
			kn_vpm_nodecmp,
			kn_vpm_nodefree);
	}
	if (KM_FAILED(result = mm_mmap(mm_kernel_context, pdt_vaddr, pdt_paddr, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, 0))) {
		return result;
	}

	free_pdt_paddr_guard.release();
	free_pdt_vaddr_guard.release();

	memset(pdt_vaddr, 0, PAGESIZE);
	context->pdt = (arch_pde_t *)pdt_vaddr;

	kn_mm_copy_global_mappings(context, mm_kernel_context);

	// kn_mm_copy_global_mappings(context, mm_kernel_context);
	return KM_RESULT_OK;
}

void mm_free_context(mm_context_t *context) {
	for (uint16_t i = 0; i < PDX_MAX; ++i) {
		arch_pde_t *pde = &(context->pdt[i]);
		if (pde->mask & PDE_P)
			mm_pgfree(UNPGADDR(pde->address));
	}

	// Free VPD query tree and pools.
	for (size_t i = 0; i < PBOS_ARRAYSIZE(context->uspace_vpm_query_tree); ++i) {
		kf_rbtree_free(&context->uspace_vpm_query_tree[i]);
	}
	for (kn_mm_vpm_poolpg_t *i = context->uspace_vpm_poolpg_list; i; i = i->header.next) {
		void *paddr = mm_getmap(mm_kernel_context, i, NULL);
		mm_unmmap(mm_kernel_context, i, PAGESIZE, 0);
		mm_pgfree(paddr);
	}

	mm_pgfree(context->pdt);
}

void mm_switch_context(mm_context_t *context) {
	kd_assert(context);
	mm_context_t *prev_context = mm_cur_contexts[ps_get_cur_euid()];
	mm_cur_contexts[ps_get_cur_euid()] = context;
	kn_mm_sync_global_mappings(prev_context);
	arch_lpdt(PGROUNDDOWN(hn_getmap(mm_kernel_context->pdt, context->pdt, NULL)));
}

PBOS_EXTERN_C_END
