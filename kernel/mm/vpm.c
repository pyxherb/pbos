#include <pbos/kn/km/mm.h>
#include <string.h>

kn_mm_vpm_poolpg_t *hn_kspace_vpm_poolpg_list;

void *kn_rounddown_to_page_leveled_addr(const void *const addr, int level) {
	if(level > KN_MM_VPM_LEVEL_MAX)
		km_panic("Invalid page rounddown level: %d", level);
	return (void *)kn_mm_vpm_rounddowners[level]((uintptr_t)addr);
}

bool kn_vpm_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	hn_vpm_t *_x = PBOS_CONTAINER_OF(hn_vpm_t, node_header, x),
			 *_y = PBOS_CONTAINER_OF(hn_vpm_t, node_header, y);

	return _x->addr < _y->addr;
}

void kn_vpm_nodefree(kf_rbtree_node_t *p) {
	hn_vpm_t *_p = PBOS_CONTAINER_OF(hn_vpm_t, node_header, p);
	// No need to release here, the VPMs is managed by `kn_mm_free_vpm` and `kn_mm_free_vpm_unchecked`.
}

hn_vpm_t *kn_mm_lookup_vpm(mm_context_t *context, const void *addr, int level) {
	kf_rbtree_t *query_tree = kn_mm_get_vpm_lookup_tree(context, addr, level);

	hn_vpm_t query_desc;

	query_desc.addr = (void *)addr;

	kf_rbtree_node_t *node;
	if (!(node = kf_rbtree_find(query_tree, &query_desc.node_header))) {
		return NULL;
	}

	return PBOS_CONTAINER_OF(hn_vpm_t, node_header, node);
}

km_result_t kn_mm_insert_vpm(mm_context_t *context, const void *addr) {
	if (kn_mm_lookup_vpm(context, addr, KN_MM_VPM_LEVEL_MAX)) {
		return KM_MAKEERROR(KM_RESULT_EXISTED);
	}

	bool inserted_flags_per_level[KN_MM_VPM_LEVEL_MAX + 1];
	memset(inserted_flags_per_level, 0, sizeof(inserted_flags_per_level));
	for (int i = 0; i <= KN_MM_VPM_LEVEL_MAX; ++i) {
		void *aligned_addr = kn_rounddown_to_page_leveled_addr(addr, i);

		hn_vpm_t *cur_level_vpm = kn_mm_lookup_vpm(context, aligned_addr, i);
		if (!cur_level_vpm) {
			km_result_t result = kn_mm_insert_vpm_unchecked(context, aligned_addr, i);

			if (KM_FAILED(result)) {
				for (int j = 0; j < i; ++j) {
					if (inserted_flags_per_level[j]) {
						kn_mm_free_vpm_unchecked(context, kn_rounddown_to_page_leveled_addr(addr, j), j);
					}
				}
				return result;
			}

			inserted_flags_per_level[i] = true;
			cur_level_vpm = kn_mm_lookup_vpm(context, aligned_addr, i);
			kd_assert(cur_level_vpm);
		}

		++cur_level_vpm->subref_count;
	}

	return KM_RESULT_OK;
}

km_result_t kn_mm_insert_vpm_unchecked(mm_context_t *context, const void *const addr, int level) {
	// Check if the address is page-aligned.
	kd_assert(!(((uintptr_t)addr) % hn_vpm_level_size[level]));

	kf_rbtree_t *query_tree = kn_mm_get_vpm_lookup_tree(context, addr, level);
	kn_mm_vpm_poolpg_t **pool_list = kn_mm_get_vpm_pool_list(context, addr, level);

	km_result_t result;
	hn_vpm_t *vpm = kn_mm_alloc_vpm_slot(context, addr, level);

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
		kn_mm_vpm_poolpg_t *newpool = new_vpmpool_vaddr;

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

void kn_mm_free_vpm(mm_context_t *context, const void *addr) {
	for (int i = 0; i <= KN_MM_VPM_LEVEL_MAX; ++i) {
		void *aligned_addr = kn_rounddown_to_page_leveled_addr(addr, i);

#ifndef _NDEBUG
		hn_vpm_t *cur_level_vpm = kn_mm_lookup_vpm(context, aligned_addr, i);
		kd_assert(cur_level_vpm);
#endif

		kn_mm_free_vpm_unchecked(context, aligned_addr, i);
	}
}

void kn_mm_free_vpm_unchecked(mm_context_t *context, const void *addr, int level) {
	kf_rbtree_t *query_tree = kn_mm_get_vpm_lookup_tree(context, addr, level);
	kn_mm_vpm_poolpg_t **pool_list = kn_mm_get_vpm_pool_list(context, addr, level);

	hn_vpm_t query_desc,
		*target_desc;

	query_desc.addr = (void *)addr;

	kf_rbtree_node_t *target_node = kf_rbtree_find(query_tree, &query_desc.node_header);
	kd_assert(target_node);
	target_desc = PBOS_CONTAINER_OF(hn_vpm_t, node_header, target_node);

	if (!(--target_desc->subref_count)) {
		if(level < KN_MM_VPM_LEVEL_MAX) {
			mm_unmmap(
				context,
				kn_lookup_pgdir_mapped_addr(kn_lookup_pgdir(context, addr, level)),
				PAGESIZE,
				0);
			kn_mm_free_pgdir(context, (void*)addr, 0);
		}
		kf_rbtree_remove(query_tree, &target_desc->node_header);

		target_desc->flags &= ~KM_MM_VPM_ALLOC;

		kn_mm_vpm_poolpg_t *pool = (kn_mm_vpm_poolpg_t *)PGFLOOR(target_desc);

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

hn_vpm_t *kn_mm_alloc_vpm_slot(mm_context_t *context, const void *addr, int level) {
	kf_rbtree_t *query_tree = kn_mm_get_vpm_lookup_tree(context, addr, level);
	kn_mm_vpm_poolpg_t **pool_list = kn_mm_get_vpm_pool_list(context, addr, level);

	for (kn_mm_vpm_poolpg_t *i = *pool_list;
		 i; i = i->header.next) {
		if (i->header.used_num == PBOS_ARRAYSIZE(i->descs))
			continue;
		for (size_t j = 0; j < PBOS_ARRAYSIZE(i->descs); ++j) {
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
