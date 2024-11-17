#include "../mm.h"
#include "../proc.h"

mm_context_t hn_kernel_mmctxt;
mm_context_t *mm_kernel_context = &hn_kernel_mmctxt;
mm_context_t **mm_current_contexts;

bool hn_vpd_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	mm_vpd_t *_x = PB_CONTAINER_OF(mm_vpd_t, node_header, x),
			 *_y = PB_CONTAINER_OF(mm_vpd_t, node_header, y);

	return _x->addr < _y->addr;
}

void hn_vpd_nodefree(kf_rbtree_node_t *p) {
	mm_vpd_t *_p = PB_CONTAINER_OF(mm_vpd_t, node_header, p);

	_p->flags &= ~MM_VPD_ALLOC;
}

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

mm_vpd_t *hn_mm_lookup_vpd(mm_context_t *context, const void *addr) {
	mm_vpd_t query_desc;

	query_desc.addr = (void *)addr;

	kf_rbtree_node_t *node;
	if (!(node = kf_rbtree_find(&context->vpd_rbtree, &query_desc.node_header))) {
		return NULL;
	}

	return PB_CONTAINER_OF(mm_vpd_t, node_header, node);
}

mm_vpd_t *hn_mm_alloc_vpd_slot(mm_context_t *context) {
	for (mm_vpdpool_t *i = context->vpd_pools; i; i = i->header.next) {
		if (i->header.used_num == PB_ARRAYSIZE(i->descs))
			continue;
		for (size_t j = 0; j < PB_ARRAYSIZE(i->descs); ++j) {
			mm_vpd_t *cur_vpd = &i->descs[j];

			if (cur_vpd->flags & MM_VPD_ALLOC) {
				continue;
			}

			++i->header.used_num;
			cur_vpd->flags = MM_VPD_ALLOC;

			return cur_vpd;
		}
	}

	return NULL;
}

km_result_t hn_mm_insert_vpd(mm_context_t *context, const void *addr) {
	mm_vpd_t query_desc;

	query_desc.addr = (void *)addr;

	if (kf_rbtree_find(&context->vpd_rbtree, &query_desc.node_header)) {
		return KM_RESULT_EXISTED;
	}

	return hn_mm_insert_vpd_unchecked(context, addr);
}

km_result_t hn_mm_insert_vpd_unchecked(mm_context_t *context, const void *addr) {
	// Check if the address is page-aligned.
	assert(!(((uintptr_t)addr) & PGOFF_MAX));

	km_result_t result;
	mm_vpd_t *vpd = hn_mm_alloc_vpd_slot(context);

	if (vpd) {
		vpd->addr = (void *)addr;
		result = kf_rbtree_insert(&context->vpd_rbtree, &vpd->node_header);
		assert(result);
		return KM_RESULT_OK;
	}

	void *new_vpdpool_paddr = NULL,
		 *new_vpdpool_vaddr = NULL;

	if ((!(new_vpdpool_vaddr = mm_kvmalloc(mm_kernel_context, PAGESIZE, PAGE_READ | PAGE_WRITE, 0)))) {
		result = KM_RESULT_NO_MEM;
		goto fail;
	}

	if (!(new_vpdpool_paddr = mm_pgalloc(MM_PMEM_AVAILABLE))) {
		result = KM_RESULT_NO_MEM;
		goto fail;
	}

	if (KM_FAILED(result = mm_mmap(mm_kernel_context, new_vpdpool_vaddr, new_vpdpool_paddr, PAGESIZE, PAGE_READ | PAGE_WRITE, 0))) {
		goto fail;
	}

	{
		mm_vpdpool_t *newpool = new_vpdpool_vaddr;

		memset(newpool, 0, sizeof(mm_vpdpool_t));

		vpd = &newpool->descs[0];
		vpd->addr = new_vpdpool_vaddr;
		vpd->flags = MM_VPD_ALLOC;

		if (context->vpd_pools)
			context->vpd_pools->header.prev = newpool;
		newpool->header.next = context->vpd_pools;

		context->vpd_pools = newpool;

		result = kf_rbtree_insert(&context->vpd_rbtree, &vpd->node_header);
		assert(result);
	}

	return KM_RESULT_OK;

fail:
	if (new_vpdpool_paddr) {
		mm_pgfree(new_vpdpool_paddr);
	}
	if (new_vpdpool_vaddr) {
		mm_vmfree(mm_kernel_context, new_vpdpool_vaddr, PAGESIZE);
	}

	return result;
}

void hn_mm_free_vpd(mm_context_t *context, const void *addr) {
	mm_vpd_t query_desc,
		*target_desc;

	query_desc.addr = (void *)addr;

	kf_rbtree_node_t *target_node = kf_rbtree_find(&context->vpd_rbtree, &query_desc.node_header);
	assert(target_node);
	target_desc = PB_CONTAINER_OF(mm_vpd_t, node_header, target_node);

	assert(target_desc);

	kf_rbtree_remove(&context->vpd_rbtree, &target_desc->node_header);

	mm_vpdpool_t *pool = (mm_vpdpool_t *)PGFLOOR(target_desc);

	target_desc->flags &= ~MM_VPD_ALLOC;

	if (!(--pool->header.used_num)) {
		if (pool == context->vpd_pools) {
			context->vpd_pools = pool->header.next;
		}

		if (pool->header.prev)
			pool->header.prev->header.next = pool->header.next;
		if (pool->header.next)
			pool->header.next->header.prev = pool->header.prev;

		mm_pgfree(pool);
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

	kf_rbtree_init(
		&context->vpd_rbtree,
		hn_vpd_nodecmp,
		hn_vpd_nodefree);
	if (KM_FAILED(result = mm_mmap(mm_kernel_context, pdt_vaddr, pdt_paddr, PAGESIZE, PAGE_READ | PAGE_WRITE, 0))) {
		goto fail;
	}
	context->pdt = pdt_vaddr;

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
	kf_rbtree_free(&context->vpd_rbtree);
	for (mm_vpdpool_t *i = context->vpd_pools; i; i = i->header.next) {
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
