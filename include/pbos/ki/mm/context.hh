#ifndef _PBOS_KI_MM_CONTEXT_HH_
#define _PBOS_KI_MM_CONTEXT_HH_

#include <pbos/kfxx/set.hh>
#include <pbos/ps/mutex.hh>
#include "kima.hh"

struct ki_mm_pmgroup_t;

struct ki_mm_pmgroup_t {
	ps::mutex_t mutex;

	kima_allocator_t kima_allocator;
	kfxx::set_t<mm_vmr_t *> vmrs;

	size_t ref_count = 0;

	PBOS_PRIVATE ki_mm_pmgroup_t(kfxx::allocator_t *allocator);
	PBOS_PRIVATE ~ki_mm_pmgroup_t();
};

PBOS_PRIVATE ki_mm_pmgroup_t *ki_mm_alloc_pmgroup();
PBOS_PRIVATE void ki_mm_destroy_pmgroup(ki_mm_pmgroup_t *pmgroup);
PBOS_PRIVATE km_result_t ki_mm_add_vmr_to_pmgroup(ki_mm_pmgroup_t *pmgroup, mm_vmr_t *vmr);
PBOS_PRIVATE bool ki_mm_remove_vmr_from_pmgroup(ki_mm_pmgroup_t *pmgroup, mm_vmr_t *vmr);

using ki_mm_vmr_tree_t = kfxx::rbtree_t<void *>;

/// @brief The virtual memory region (VMR) structure, manages once user-space mapping.
typedef struct _mm_vmr_t
	: public ki_mm_vmr_tree_t::node_t {
	ps::mutex_t mutex;

	mm_vmr_t *prev, *next;
	mm_context_t *mm_context = nullptr;

	ki_mm_pmgroup_t *default_pmgroup = nullptr;

	size_t size = 0;
	mm_pgaccess_t access = 0;
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

	_mm_context_t();
} mm_context_t;

extern mm_context_t **mm_cur_contexts;

#endif
