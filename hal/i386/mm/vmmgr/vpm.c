#include <pbos/km/logger.h>
#include "../../mm.h"

hn_vpm_poolpg_t *hn_kspace_vpm_poolpg_list;
kf_rbtree_t hn_kspace_vpm_query_tree;

bool hn_vpm_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	hn_vpm_t *_x = PB_CONTAINER_OF(hn_vpm_t, node_header, x),
			 *_y = PB_CONTAINER_OF(hn_vpm_t, node_header, y);

	return _x->addr < _y->addr;
}

void hn_vpm_nodefree(kf_rbtree_node_t *p) {
	hn_vpm_t *_p = PB_CONTAINER_OF(hn_vpm_t, node_header, p);
}


hn_vpm_t *hn_mm_lookup_vpm(mm_context_t *context, const void *addr) {
	hn_vpm_t query_desc;

	query_desc.addr = (void *)addr;

	kf_rbtree_node_t *node;
	if (!(node = kf_rbtree_find(&context->uspace_vpm_query_tree, &query_desc.node_header))) {
		return NULL;
	}

	return PB_CONTAINER_OF(hn_vpm_t, node_header, node);
}

hn_vpm_t *hn_mm_alloc_vpm_slot(mm_context_t *context, const void *addr) {
	kf_rbtree_t *query_tree =
		addr >= (void *)KSPACE_VBASE
			? &hn_kspace_vpm_query_tree
			: &context->uspace_vpm_query_tree;
	hn_vpm_poolpg_t **pool_list =
		addr >= (void *)KSPACE_VBASE
			? &hn_kspace_vpm_poolpg_list
			: &context->uspace_vpm_poolpg_list;

	for (hn_vpm_poolpg_t *i = *pool_list;
		 i; i = i->header.next) {
		if (i->header.used_num == PB_ARRAYSIZE(i->descs))
			continue;
		for (size_t j = 0; j < PB_ARRAYSIZE(i->descs); ++j) {
			hn_vpm_t *cur_vpm = &i->descs[j];

			if (cur_vpm->flags & HN_VPM_ALLOC) {
				continue;
			}

			++i->header.used_num;
			if (cur_vpm->node_header.l)
				kprintf("l: %p", cur_vpm->node_header.l);
			if (cur_vpm->node_header.r)
				kprintf("r: %p", cur_vpm->node_header.r);
			if (cur_vpm->node_header.p)
				;
			assert(!cur_vpm->node_header.l);
			assert(!cur_vpm->node_header.r);
			assert(!cur_vpm->node_header.p);
			memset(cur_vpm, 0, sizeof(*cur_vpm));
			cur_vpm->flags = HN_VPM_ALLOC;
			cur_vpm->addr = (void *)0xcdcdcdcd;

			return cur_vpm;
		}
	}

	return NULL;
}

km_result_t hn_mm_insert_vpm(mm_context_t *context, const void *addr) {
	hn_vpm_t query_desc;

	if (addr == 0x82000000)
		kputs("Caution");

	query_desc.addr = (void *)addr;

	if (kf_rbtree_find(
			addr >= (void *)KSPACE_VBASE
				? &hn_kspace_vpm_query_tree
				: &context->uspace_vpm_query_tree,
			&query_desc.node_header)) {
		return KM_RESULT_EXISTED;
	}

	return hn_mm_insert_vpm_unchecked(context, addr);
}

km_result_t hn_mm_insert_vpm_unchecked(mm_context_t *context, const void *const addr) {
	// Check if the address is page-aligned.
	assert(!(((uintptr_t)addr) & PGOFF_MAX));

	kf_rbtree_t *query_tree =
		addr >= (void *)KSPACE_VBASE
			? &hn_kspace_vpm_query_tree
			: &context->uspace_vpm_query_tree;
	hn_vpm_poolpg_t **pool_list =
		addr >= (void *)KSPACE_VBASE
			? &hn_kspace_vpm_poolpg_list
			: &context->uspace_vpm_poolpg_list;

	km_result_t result;
	hn_vpm_t *vpm = hn_mm_alloc_vpm_slot(context, addr);

	if (vpm) {
		// kprintf("VPM: %p\n", vpm->addr);
		vpm->addr = (void *)addr;
		kf_rbtree_node_t *r;
		if ((r = kf_rbtree_find(query_tree, &vpm->node_header))) {
			kputs("Warning");
			kprintf("%p\n", addr);
			kprintf("%p\n", vpm->addr);
			hn_vpm_t *rr = PB_CONTAINER_OF(hn_vpm_t, node_header, r);
			assert(rr != vpm);
			kprintf("%p\n", rr->addr);
		}
		result = kf_rbtree_insert(
			query_tree,
			&vpm->node_header);
		assert(KM_SUCCEEDED(result));
		return KM_RESULT_OK;
	}

	void *new_vpmpool_paddr = NULL,
		 *new_vpmpool_vaddr = NULL;

	if ((!(new_vpmpool_vaddr = mm_kvmalloc(mm_kernel_context, PAGESIZE, PAGE_READ | PAGE_WRITE, VMALLOC_NORESERVE)))) {
		result = KM_RESULT_NO_MEM;
		goto fail;
	}

	if (!(new_vpmpool_paddr = mm_pgalloc(MM_PMEM_AVAILABLE))) {
		result = KM_RESULT_NO_MEM;
		goto fail;
	}

	if (KM_FAILED(result = mm_mmap(mm_kernel_context, new_vpmpool_vaddr, new_vpmpool_paddr, PAGESIZE, PAGE_READ | PAGE_WRITE, MMAP_NOSETVPM))) {
		goto fail;
	}

	{
		hn_vpm_poolpg_t *newpool = new_vpmpool_vaddr;

		assert(newpool != addr);

		memset(newpool, 0, PAGESIZE);

		newpool->header.used_num = 1;  // Self VPM is not included

		// Initialize the self VPM.
		vpm = &newpool->descs[0];
		vpm->addr = new_vpmpool_vaddr;
		vpm->flags = HN_VPM_ALLOC;
		result = kf_rbtree_insert(query_tree, &vpm->node_header);
		assert(KM_SUCCEEDED(result));

		if (vpm->addr == 0x82000000)
			kputs("Caution");

		// Initialize the target VPM.
		vpm = &newpool->descs[1];
		vpm->addr = (void *)addr;
		vpm->flags = HN_VPM_ALLOC;
		result = kf_rbtree_insert(query_tree, &vpm->node_header);
		assert(KM_SUCCEEDED(result));

		if (vpm->addr == 0x82000000)
			kputs("Caution");

		if (*pool_list)
			(*pool_list)->header.prev = newpool;
		newpool->header.next = *pool_list;

		*pool_list = newpool;
	}

	return KM_RESULT_OK;

fail:
	if (new_vpmpool_paddr) {
		mm_pgfree(new_vpmpool_paddr);
	}
	if (new_vpmpool_vaddr) {
		mm_vmfree(mm_kernel_context, new_vpmpool_vaddr, PAGESIZE);
	}

	return result;
}

void hn_mm_free_vpm(mm_context_t *context, const void *addr) {
	hn_vpm_t query_desc,
		*target_desc;

	query_desc.addr = (void *)addr;

	kf_rbtree_t *query_tree =
		addr >= (void *)KSPACE_VBASE
			? &hn_kspace_vpm_query_tree
			: &context->uspace_vpm_query_tree;
	hn_vpm_poolpg_t **pool_list =
		addr >= (void *)KSPACE_VBASE
			? &hn_kspace_vpm_poolpg_list
			: &context->uspace_vpm_poolpg_list;

	kf_rbtree_node_t *target_node = kf_rbtree_find(query_tree, &query_desc.node_header);
	assert(target_node);
	target_desc = PB_CONTAINER_OF(hn_vpm_t, node_header, target_node);

	assert(target_desc);

	kf_rbtree_remove(query_tree, &target_desc->node_header);

	target_desc->flags &= ~HN_VPM_ALLOC;

	hn_vpm_poolpg_t *pool = (hn_vpm_poolpg_t *)PGFLOOR(target_desc);

	if (!(--pool->header.used_num)) {
		query_desc.addr = pool;
		if (query_desc.addr == 0x82000000)
			kputs("Caution");
		kf_rbtree_remove(query_tree, kf_rbtree_find(query_tree, &query_desc.node_header));

		if (pool == *pool_list) {
			*pool_list = pool->header.next;
		}

		if (pool->header.prev)
			pool->header.prev->header.next = pool->header.next;
		if (pool->header.next)
			pool->header.next->header.prev = pool->header.prev;

		mm_pgfree(mm_getmap(context, pool));
		mm_unmmap(context, pool, PAGESIZE, MMAP_NOSETVPM);
	}
}
