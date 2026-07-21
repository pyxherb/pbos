#ifndef _PBOS_KI_MM_CONTEXT_HH_
#define _PBOS_KI_MM_CONTEXT_HH_

#include <pbos/kfxx/radix_map.hh>
#include <pbos/kfxx/set.hh>
#include <pbos/ps/mutex.hh>
#include <pbos/ps/semaphore.hh>
#include "kima.hh"

struct ki_mm_rmlt_t;

struct ki_mm_rmlt_t {
	ps::mutex_t mutex;

	kima_allocator_t kima_allocator;
	kfxx::set_t<mm_vmr_t *> vmrs;

	size_t ref_count = 0;

	PBOS_PRIVATE ki_mm_rmlt_t(kfxx::allocator_t *allocator);
	PBOS_PRIVATE ~ki_mm_rmlt_t();
};

PBOS_PRIVATE ki_mm_rmlt_t *ki_mm_alloc_rmlt();
PBOS_PRIVATE void ki_mm_destroy_rmlt(ki_mm_rmlt_t *rmlt);
PBOS_PRIVATE km_result_t ki_mm_add_vmr_to_rmlt(ki_mm_rmlt_t *rmlt, mm_vmr_t *vmr);
PBOS_PRIVATE bool ki_mm_remove_vmr_from_rmlt(ki_mm_rmlt_t *rmlt, mm_vmr_t *vmr);

using ki_mm_vmr_tree_t = kfxx::rbtree_t<void *>;
using ki_mm_revmap_set_tree_t = kfxx::rbtree_t<void *>;

struct ki_related_vmrs_entry_t {
	ki_mm_revmap_set_tree_t vmr_index_tree;
	ps::semaphore_t vmr_index_tree_semaphore;
};

struct ki_reversal_map_t {
	kfxx::radix_map_t<ps_proc_id_t, ki_related_vmrs_entry_t, 3> proc_related_vmrs;
	ps::semaphore_t related_vmrs_semaphore;

	PBOS_FORCEINLINE ki_reversal_map_t(kfxx::allocator_t *allocator) : proc_related_vmrs(allocator) {}
};

extern kfxx::option_t<kfxx::radix_map_t<uintptr_t, ki_reversal_map_t, 3>> ki_reversal_mapping;
extern ps::semaphore_t ki_reversal_mapping_semaphore;

struct ki_vmr_index_t : public ki_mm_revmap_set_tree_t::node_t {
	mm_vmr_t *vmr;
};

/// @brief The virtual memory region (VMR) structure, manages one user-space mapping.
typedef struct _mm_vmr_t
	: public ki_mm_vmr_tree_t::node_t {
	ps::mutex_t mutex;

	mm_context_t *mm_context = nullptr;

	size_t size = 0;
	mm_page_access_t access = 0;

	ki_vmr_index_t *indices = nullptr;
	uint8_t *index_alloc_masks = nullptr;
	size_t index_alloc_cur_index = 0;

	PBOS_FORCEINLINE bool get_index_alloc_mask(size_t i) const {
		return (index_alloc_masks[(i >> 3)] >> (i & 7)) & 1;
	}

	PBOS_FORCEINLINE void set_index_alloc_mask(size_t i) {
		index_alloc_masks[(i >> 3)] |= (1 << (i & 7));
	}

	PBOS_FORCEINLINE void clear_index_alloc_mask(size_t i) {
		index_alloc_masks[(i >> 3)] &= ~(1 << (i & 7));
	}
} mm_vmr_t;

PBOS_PRIVATE void ki_mm_destroy_vmr(mm_vmr_t *vmr);

typedef struct _mm_context_t {
	ps_pcb_t *pcb = nullptr;

	/// @brief An opaque pointer to the platform-specific page table.
	void *page_table = nullptr;

	/// @brief KIMA pool for common structures.
	kfxx::option_t<kima_pool_t> kima_common_pool;

	/// @brief KIMA pool for VMR structures.
	kfxx::option_t<kima_pool_t> kima_vmr_pool;
	/// @brief The VMR tree.
	ki_mm_vmr_tree_t vmr_tree;

	ps::rec_mutex_t vmr_mutex;

	size_t num_reserved_user_pages;

	ps::semaphore_t user_page_reserve_quota_semaphore;

	_mm_context_t();
} mm_context_t;

enum {
	KI_REVERSAL_MAP_GRANULE = 128,
};

extern mm_context_t **mm_cur_contexts;

#endif
