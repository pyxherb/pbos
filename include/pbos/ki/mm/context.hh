#ifndef _PBOS_KI_MM_CONTEXT_HH_
#define _PBOS_KI_MM_CONTEXT_HH_

#include "kima.hh"

typedef struct _ps_vmr_t : kfxx::rbtree_t<void *>::node_t {
	size_t size = 0;
	mm_pgaccess_t access = 0;
	bool rescan_needed = false;

	_ps_vmr_t();
} ps_vmr_t;

typedef struct _mm_context_t {
	/// @brief An opaque pointer to the platform-specific page table.
	void *page_table = nullptr;

	/// @brief KIMA pool for common structures.
	kima_pool_t kima_common_pool;

	/// @brief KIMA pool for VMR structures.
	kima_pool_t kima_vmr_pool;
	/// @brief The VMR tree.
	kfxx::rbtree_t<void *> vmr_tree;

	~_mm_context_t();
} mm_context_t;

extern mm_context_t **mm_cur_contexts;

#endif
