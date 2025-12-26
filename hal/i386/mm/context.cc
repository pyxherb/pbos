#include <pbos/hal/irq.hh>
#include <pbos/kfxx/scope_guard.hh>
#include "../mm.hh"
#include "../proc.hh"

PBOS_EXTERN_C_BEGIN

mm_context_t hn_kernel_mm_context;
mm_context_t *mm_kernel_context = &hn_kernel_mm_context;
mm_context_t **mm_cur_contexts = nullptr;

void kn_mm_copy_global_mappings(mm_context_t *dest, const mm_context_t *src) {
	if (dest == src)
		return;
	memcpy(
		dest->pdt + PDX(KBOTTOM_VBASE),
		src->pdt + PDX(KBOTTOM_VBASE),
		sizeof(arch_pde_t) * PDX(KBOTTOM_SIZE));
	memcpy(
		dest->pdt + PDX(KERNEL_VBASE),
		src->pdt + PDX(KERNEL_VBASE),
		sizeof(arch_pde_t) * PDX((PDX_MAX + 1) - KERNEL_VBASE));
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
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	}
	kfxx::scope_guard free_pdt_paddr_guard([pdt_paddr]() noexcept {
		mm_pgfree(pdt_paddr);
	});

	if (!(pdt_vaddr = mm_kvmalloc(mm_get_cur_context(), PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, 0))) {
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	}
	kfxx::scope_guard free_pdt_vaddr_guard([pdt_vaddr]() noexcept {
		mm_vmfree(mm_get_cur_context(), pdt_vaddr, PAGESIZE);
	});

	kfxx::rbtree_t<void *> *vpm_query_tree;

	if (!(vpm_query_tree = (kfxx::rbtree_t<void *> *)mm_kmalloc(
			  sizeof(kfxx::rbtree_t<void *>) * kn_cur_paging_config->pgtab_level,
			  alignof(kfxx::rbtree_t<void *>)))) {
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	}

	for (size_t i = 0; i < kn_cur_paging_config->pgtab_level; ++i) {
		kfxx::construct_at<kfxx::rbtree_t<void *>>(vpm_query_tree + i);
	}

	kfxx::scope_guard free_vpm_query_tree_guard([vpm_query_tree]() noexcept {
		for (size_t i = 0; i < kn_cur_paging_config->pgtab_level; ++i) {
			kfxx::destroy_at<kfxx::rbtree_t<void *>>(vpm_query_tree + i);
		}
		mm_kfree(vpm_query_tree);
	});

	kfxx::construct_at<mm_context_t>(context);
	if (KM_FAILED(result = mm_mmap(mm_get_cur_context(), pdt_vaddr, pdt_paddr, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, 0))) {
		return result;
	}

	free_vpm_query_tree_guard.release();
	free_pdt_paddr_guard.release();
	free_pdt_vaddr_guard.release();

	context->uspace_vpm_query_tree = vpm_query_tree;
	memset(pdt_vaddr, 0, PAGESIZE);
	context->pdt = (arch_pde_t *)pdt_vaddr;

	kn_mm_copy_global_mappings(context, mm_get_cur_context());

	// Map all valid page tables.
	// See init.cc for initial mapping code.
	for (size_t i = 0; i < PDX_MAX; ++i) {
		// One PDE manages 4MB.
		arch_pde_t *pde = &context->pdt[i];
		if (pde->mask & PDE_P) {
			void *target_ptr = ((char *)KALLPGTAB_VBASE) + PAGESIZE * i;
			arch_pde_t *target_pde = &context->pdt[PDX(target_ptr)];

			void *vptr = mm_kvmalloc(mm_get_cur_context(), PAGESIZE, PAGE_READ | PAGE_WRITE, 0);
			if (!vptr)
				return KM_RESULT_NO_MEM;
			km_unwrap_result(mm_mmap(mm_get_cur_context(), vptr, UNPGADDR(target_pde->address), PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, 0));

			kfxx::oneshot_scope_guard sg([vptr]() noexcept {
				mm_unmmap(mm_get_cur_context(), vptr, PAGESIZE, 0);
			});

			arch_pte_t *mapped_pt = (arch_pte_t *)vptr;

			mapped_pt[PTX(target_ptr)].address = pde->address;
			mapped_pt[PTX(target_ptr)].mask = PTE_P | PTE_RW;
		}
	}

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
	for (size_t i = 0; i < kn_cur_paging_config->pgtab_level; ++i) {
		context->uspace_vpm_query_tree[i].clear([](kfxx::rbtree_t<void *>::node_t *node) noexcept {
			kn_vpm_nodefree(static_cast<hn_vpm_t *>(node));
		});
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
	io::irq_disable_lock irq_lock;

	mm_context_t *prev_context = mm_get_cur_context();
	mm_cur_contexts[ps_get_cur_euid()] = context;
	kn_mm_copy_global_mappings(context, prev_context);
	// kn_mm_copy_global_mappings(mm_kernel_context, prev_context);
	asm volatile("xchg %bx, %bx");
	arch_lpdt(PGROUNDDOWN(mm_getmap(prev_context, context->pdt, NULL)));
}

PBOS_EXTERN_C_END
