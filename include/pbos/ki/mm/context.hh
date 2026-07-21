#ifndef _PBOS_KI_MM_CONTEXT_HH_
#define _PBOS_KI_MM_CONTEXT_HH_

#include <pbos/kfxx/set.hh>
#include <pbos/ps/mutex.hh>
#include "kima.hh"
#include <pbos/ps/semaphore.hh>

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

/// @brief The virtual memory region (VMR) structure, manages once user-space mapping.
typedef struct _mm_vmr_t
	: public ki_mm_vmr_tree_t::node_t {
	ps::mutex_t mutex;

	mm_vmr_t *prev, *next;
	mm_context_t *mm_context = nullptr;

	ki_mm_rmlt_t *default_rmlt = nullptr;

	size_t size = 0;
	mm_page_access_t access = 0;
} mm_vmr_t;

PBOS_PRIVATE void ki_mm_destroy_vmr(mm_vmr_t *vmr);

typedef struct _mm_context_t {
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

extern mm_context_t **mm_cur_contexts;

#endif
