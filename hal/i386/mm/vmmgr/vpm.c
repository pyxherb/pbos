#include <pbos/km/logger.h>
#include "../../mm.h"

static uintptr_t hn_level0_rounddowner(uintptr_t addr) {
	return addr & (uintptr_t)VADDR(PDX_MAX, 0, 0);
}

static uintptr_t hn_level1_rounddowner(uintptr_t addr) {
	return addr & (uintptr_t)VADDR(PDX_MAX, PTX_MAX, 0);
}

hn_vpm_poolpg_t *hn_kspace_vpm_poolpg_list;

size_t kn_mm_vpm_level_max = HN_VPM_LEVEL_MAX;
kf_rbtree_t hn_kspace_vpm_query_tree[HN_VPM_LEVEL_MAX + 1];
size_t hn_vpm_level_size[] = {
	(size_t)VADDR(1, 0, 0),
	(size_t)VADDR(0, 1, 0)
};
hn_vpm_level_rounddowner_t kn_mm_vpm_rounddowners[] = {
	hn_level0_rounddowner,
	hn_level1_rounddowner
};

kf_rbtree_t *hn_mm_get_vpm_lookup_tree(mm_context_t *context, const void *addr, int level) {
	return ISINRANGE(USPACE_VBASE, USPACE_SIZE, addr)
			   ? &context->uspace_vpm_query_tree[level]
			   : &hn_kspace_vpm_query_tree[level];
}

hn_vpm_poolpg_t **hn_mm_get_vpm_pool_list(mm_context_t *context, const void *addr, int level) {
	return ISINRANGE(USPACE_VBASE, USPACE_SIZE, addr)
			   ? &context->uspace_vpm_poolpg_list
			   : &hn_kspace_vpm_poolpg_list;
}

bool hn_vpm_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	hn_vpm_t *_x = PB_CONTAINER_OF(hn_vpm_t, node_header, x),
			 *_y = PB_CONTAINER_OF(hn_vpm_t, node_header, y);

	return _x->addr < _y->addr;
}

void hn_vpm_nodefree(kf_rbtree_node_t *p) {
	hn_vpm_t *_p = PB_CONTAINER_OF(hn_vpm_t, node_header, p);
	// No need to release here, the VPMs is managed by `hn_mm_free_vpm` and `hn_mm_free_vpm_unchecked`.
}

hn_vpm_t *hn_mm_lookup_vpm(mm_context_t *context, const void *addr, int level) {
	kf_rbtree_t *query_tree = hn_mm_get_vpm_lookup_tree(context, addr, level);

	hn_vpm_t query_desc;

	query_desc.addr = (void *)addr;

	kf_rbtree_node_t *node;
	if (!(node = kf_rbtree_find(query_tree, &query_desc.node_header))) {
		return NULL;
	}

	return PB_CONTAINER_OF(hn_vpm_t, node_header, node);
}

hn_vpm_t *hn_mm_alloc_vpm_slot(mm_context_t *context, const void *addr, int level) {
	kf_rbtree_t *query_tree = hn_mm_get_vpm_lookup_tree(context, addr, level);
	hn_vpm_poolpg_t **pool_list = hn_mm_get_vpm_pool_list(context, addr, level);

	for (hn_vpm_poolpg_t *i = *pool_list;
		 i; i = i->header.next) {
		if (i->header.used_num == PB_ARRAYSIZE(i->descs))
			continue;
		for (size_t j = 0; j < PB_ARRAYSIZE(i->descs); ++j) {
			hn_vpm_t *cur_vpm = &i->descs[j];

			if (cur_vpm->flags & KM_MM_VPM_ALLOC) {
				continue;
			}

			++i->header.used_num;
			memset(cur_vpm, 0, sizeof(*cur_vpm));
			cur_vpm->flags = KM_MM_VPM_ALLOC;
			cur_vpm->addr = (void *)0xcdcdcdcd;

			return cur_vpm;
		}
	}

	return NULL;
}

km_result_t hn_mm_insert_vpm(mm_context_t *context, const void *addr) {
	if (hn_mm_lookup_vpm(context, addr, HN_VPM_LEVEL_MAX)) {
		return KM_MAKEERROR(KM_RESULT_EXISTED);
	}

	bool inserted_flags_per_level[HN_VPM_LEVEL_MAX + 1];
	memset(inserted_flags_per_level, 0, sizeof(inserted_flags_per_level));
	for (int i = 0; i <= HN_VPM_LEVEL_MAX; ++i) {
		void *aligned_addr = kn_rounddown_to_page_leveled_addr(addr, i);

		hn_vpm_t *cur_level_vpm = hn_mm_lookup_vpm(context, aligned_addr, i);
		if (!cur_level_vpm) {
			km_result_t result = hn_mm_insert_vpm_unchecked(context, aligned_addr, i);

			if (KM_FAILED(result)) {
				for (int j = 0; j < i; ++j) {
					if (inserted_flags_per_level[j]) {
						hn_mm_free_vpm_unchecked(context, kn_rounddown_to_page_leveled_addr(addr, j), j);
					}
				}
				return result;
			}

			inserted_flags_per_level[i] = true;
			cur_level_vpm = hn_mm_lookup_vpm(context, aligned_addr, i);
			kd_assert(cur_level_vpm);
		}

		++cur_level_vpm->subref_count;
	}

	return KM_RESULT_OK;
}

km_result_t hn_mm_insert_vpm_unchecked(mm_context_t *context, const void *const addr, int level) {
	// Check if the address is page-aligned.
	kd_assert(!(((uintptr_t)addr) % hn_vpm_level_size[level]));

	kf_rbtree_t *query_tree = hn_mm_get_vpm_lookup_tree(context, addr, level);
	hn_vpm_poolpg_t **pool_list = hn_mm_get_vpm_pool_list(context, addr, level);

	km_result_t result;
	hn_vpm_t *vpm = hn_mm_alloc_vpm_slot(context, addr, level);

	if (vpm) {
		vpm->addr = (void *)addr;
		result = kf_rbtree_insert(
			query_tree,
			&vpm->node_header);
		kd_assert(KM_SUCCEEDED(result));
		return KM_RESULT_OK;
	}

	void *new_vpmpool_paddr = NULL,
		 *new_vpmpool_vaddr = NULL;

	if ((!(new_vpmpool_vaddr = mm_kvmalloc(mm_kernel_context, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, VMALLOC_NORESERVE)))) {
		result = KM_RESULT_NO_MEM;
		goto fail;
	}

	if (!(new_vpmpool_paddr = mm_pgalloc(MM_PMEM_AVAILABLE))) {
		result = KM_RESULT_NO_MEM;
		goto fail;
	}

	if (KM_FAILED(result = mm_mmap(mm_kernel_context, new_vpmpool_vaddr, new_vpmpool_paddr, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, MMAP_NOSETVPM))) {
		goto fail;
	}

	{
		hn_vpm_poolpg_t *newpool = new_vpmpool_vaddr;

		kd_assert(newpool != addr);

		memset(newpool, 0, PAGESIZE);

		newpool->header.used_num = 1;  // Self VPM is not included

		// Initialize the self VPM.
		vpm = &newpool->descs[0];
		vpm->addr = new_vpmpool_vaddr;
		vpm->flags = KM_MM_VPM_ALLOC;
		result = kf_rbtree_insert(query_tree, &vpm->node_header);
		kd_assert(KM_SUCCEEDED(result));

		// Initialize the target VPM.
		vpm = &newpool->descs[1];
		vpm->addr = (void *)addr;
		vpm->flags = KM_MM_VPM_ALLOC;
		result = kf_rbtree_insert(query_tree, &vpm->node_header);
		kd_assert(KM_SUCCEEDED(result));

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

	return KM_MAKEERROR(result);
}

void hn_mm_free_vpm(mm_context_t *context, const void *addr) {
	for (int i = 0; i <= HN_VPM_LEVEL_MAX; ++i) {
		void *aligned_addr = kn_rounddown_to_page_leveled_addr(addr, i);

#ifndef _NDEBUG
		hn_vpm_t *cur_level_vpm = hn_mm_lookup_vpm(context, aligned_addr, i);
		kd_assert(cur_level_vpm);
#endif

		hn_mm_free_vpm_unchecked(context, aligned_addr, i);
	}
}

void hn_mm_free_vpm_unchecked(mm_context_t *context, const void *addr, int level) {
	kf_rbtree_t *query_tree = hn_mm_get_vpm_lookup_tree(context, addr, level);
	hn_vpm_poolpg_t **pool_list = hn_mm_get_vpm_pool_list(context, addr, level);

	hn_vpm_t query_desc,
		*target_desc;

	query_desc.addr = (void *)addr;

	kf_rbtree_node_t *target_node = kf_rbtree_find(query_tree, &query_desc.node_header);
	kd_assert(target_node);
	target_desc = PB_CONTAINER_OF(hn_vpm_t, node_header, target_node);

	if (!(--target_desc->subref_count)) {
		switch(level) {
			case 0: {
				hn_mad_t *mad = hn_get_mad(context->pdt[PDX(addr)].address);
				kd_assert(mad);
				kd_assert(mad->exdata.mapped_pgtab_addr);
				mm_unmmap(context, UNPGADDR(mad->exdata.mapped_pgtab_addr), PAGESIZE, 0);
				kn_mm_free_pgdir(context, (void*)addr, 0);
				break;
			}
			case 1:
				break;
			default:
				km_panic("Invalid page level: %d\n", level);
		}
		kf_rbtree_remove(query_tree, &target_desc->node_header);

		target_desc->flags &= ~KM_MM_VPM_ALLOC;

		hn_vpm_poolpg_t *pool = (hn_vpm_poolpg_t *)PGFLOOR(target_desc);

		if (!(--pool->header.used_num)) {
			query_desc.addr = pool;
			kf_rbtree_remove(query_tree, kf_rbtree_find(query_tree, &query_desc.node_header));

			if (pool == *pool_list) {
				*pool_list = pool->header.next;
			}

			if (pool->header.prev)
				pool->header.prev->header.next = pool->header.next;
			if (pool->header.next)
				pool->header.next->header.prev = pool->header.prev;

			mm_pgfree(mm_getmap(context, pool, NULL));
			mm_unmmap(context, pool, PAGESIZE, MMAP_NOSETVPM);
		}
	}
}
