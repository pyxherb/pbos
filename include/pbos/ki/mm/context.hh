#ifndef _PBOS_KI_MM_CONTEXT_HH_
#define _PBOS_KI_MM_CONTEXT_HH_

#include "kima.hh"
#include <pbos/ps/mutex.hh>

typedef struct _mm_vmr_t : kfxx::RBTree<void *>::Node {
	size_t size = 0;
	mm_pgaccess_t access = 0;

	ps::Mutex mutex;
} mm_vmr_t;

using ki_mm_vmr_tree_t = kfxx::RBTree<void *>;

typedef struct _mm_context_t {
	/// @brief An opaque pointer to the platform-specific page table.
	void *page_table = nullptr;

	/// @brief KIMA pool for common structures.
	kima_pool_t kima_common_pool;

	/// @brief KIMA pool for VMR structures.
	kima_pool_t kima_vmr_pool;
	/// @brief The VMR tree.
	ki_mm_vmr_tree_t vmr_tree;

	ps::RecMutex vmr_mutex;

	_mm_context_t();
} mm_context_t;

extern mm_context_t **mm_cur_contexts;

#endif
