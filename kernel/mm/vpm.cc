#include <alloca.h>
#include <pbos/km/logger.h>
#include <string.h>
#include <pbos/hal/irq.hh>
#include <pbos/kn/km/mm.hh>

PBOS_EXTERN_C_BEGIN

const kn_paging_config_t *kn_cur_paging_config;

kn_mm_vpm_poolpg_t *hn_kspace_vpm_poolpg_list;

void *kn_rounddown_to_page_leveled_addr(const void *const addr, int level) {
	if (level >= kn_cur_paging_config->pgtab_level)
		km_panic("Invalid page rounddown level: %d", level);
	return (void *)kn_cur_paging_config->vpm_rounddowners[level]((uintptr_t)addr);
}

void kn_vpm_nodefree(hn_vpm_t *p) {
	// No need to release here, the VPMs is managed by `kn_mm_free_vpm` and `kn_mm_free_vpm_unchecked`.
}

hn_vpm_t *kn_mm_lookup_vpm(mm_context_t *context, const void *addr, int level) {
	kfxx::rbtree_t<void *> *query_tree = kn_mm_get_vpm_lookup_tree(context, addr, level);

	kfxx::rbtree_t<void *>::node_t *node;
	if (!(node = query_tree->find((void *)addr))) {
		return NULL;
	}

	return static_cast<hn_vpm_t *>(node);
}

km_result_t kn_mm_insert_vpm(mm_context_t *context, const void *addr) {
	if (kn_mm_lookup_vpm(context, addr, (kn_cur_paging_config->pgtab_level - 1))) {
		kd_assert("Inserting duplicated VPM, aborting");
	}

	size_t levels = kn_cur_paging_config->pgtab_level;
	bool *inserted_flags_per_level = (bool *)alloca(levels);
	memset(inserted_flags_per_level, 0, levels * sizeof(bool));
	for (int i = 0; i < levels; ++i) {
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
	kd_assert(!(((uintptr_t)addr) % kn_cur_paging_config->vpm_level_size[level]));

	kfxx::rbtree_t<void *> *query_tree = kn_mm_get_vpm_lookup_tree(context, addr, level);
	kn_mm_vpm_poolpg_t **pool_list = kn_mm_get_vpm_pool_list(context, addr, level);

	km_result_t result;
	hn_vpm_t *vpm = kn_mm_alloc_vpm_slot(context, addr, level);

	if (vpm) {
		vpm->rb_value = (void *)addr;
		query_tree->insert(vpm);
		return KM_RESULT_OK;
	}

	void *new_vpmpool_paddr = NULL,
		 *new_vpmpool_vaddr = NULL;

	if ((!(new_vpmpool_vaddr = mm_kvmalloc(context, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, VMALLOC_NOSETVPM)))) {
		result = KM_RESULT_NO_MEM;
		goto fail;
	}

	if (!(new_vpmpool_paddr = mm_pgalloc(MM_PMEM_AVAILABLE))) {
		result = KM_RESULT_NO_MEM;
		goto fail;
	}

	if (KM_FAILED(result = mm_mmap(context, new_vpmpool_vaddr, new_vpmpool_paddr, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, MMAP_NOSETVPM))) {
		goto fail;
	}

	{
		kn_mm_vpm_poolpg_t *newpool = (kn_mm_vpm_poolpg_t *)new_vpmpool_vaddr;

		kd_assert(newpool != addr);

		memset(newpool, 0, PAGESIZE);

		newpool->header.used_num = 1;  // Self VPM is not included

		// Initialize the self VPM.
		vpm = &newpool->descs[0];
		vpm->rb_value = new_vpmpool_vaddr;
		vpm->flags = KM_MM_VPM_ALLOC;
		query_tree->insert(vpm);
		kd_assert(KM_SUCCEEDED(result));

		// Initialize the target VPM.
		vpm = &newpool->descs[1];
		vpm->rb_value = (void *)addr;
		vpm->flags = KM_MM_VPM_ALLOC;
		query_tree->insert(vpm);
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
		mm_vmfree(context, new_vpmpool_vaddr, PAGESIZE);
	}

	return KM_MAKEERROR(result);
}

void kn_mm_free_vpm(mm_context_t *context, const void *addr) {
	for (int i = 0; i <= kn_cur_paging_config->pgtab_level - 1; ++i) {
		void *aligned_addr = kn_rounddown_to_page_leveled_addr(addr, i);

#ifndef NDEBUG
		hn_vpm_t *cur_level_vpm = kn_mm_lookup_vpm(context, aligned_addr, i);
		kd_assert(cur_level_vpm);
#endif

		kn_mm_free_vpm_unchecked(context, aligned_addr, i);
	}
}

void kn_mm_free_vpm_unchecked(mm_context_t *context, const void *addr, int level) {
	io::irq_disable_lock irq_lock;
	kfxx::rbtree_t<void *> *query_tree = kn_mm_get_vpm_lookup_tree(context, addr, level);
	kn_mm_vpm_poolpg_t **pool_list = kn_mm_get_vpm_pool_list(context, addr, level);

	hn_vpm_t *target_desc;

	kfxx::rbtree_t<void *>::node_t *target_node = query_tree->find((void *)addr);
	kd_assert(target_node);
	target_desc = static_cast<hn_vpm_t *>(target_node);

	if (!(--target_desc->subref_count)) {
		if (level < kn_cur_paging_config->pgtab_level - 1) {
			mm_unmmap(
				context,
				kn_lookup_pgdir_mapped_addr(kn_lookup_pgdir(context, (void *)addr, level)),
				PAGESIZE,
				0);
			kn_mm_free_pgdir(context, (void *)addr, 0);
		}
		query_tree->remove(target_desc);

		target_desc->flags &= ~KM_MM_VPM_ALLOC;

		kn_mm_vpm_poolpg_t *pool = (kn_mm_vpm_poolpg_t *)PGFLOOR(target_desc);

		if (!(--pool->header.used_num)) {
			query_tree->remove(query_tree->find(pool));

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
	io::irq_disable_lock irq_lock;

	kfxx::rbtree_t<void *> *query_tree = kn_mm_get_vpm_lookup_tree(context, addr, level);
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

			return cur_vpm;
		}
	}

	return NULL;
}

PBOS_EXTERN_C_END
