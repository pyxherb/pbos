#ifndef _PBOS_KI_MM_PGALLOC_HH_
#define _PBOS_KI_MM_PGALLOC_HH_

#include <pbos/hal/spinlock.h>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/generated/mm.h>
#include "context.hh"

PBOS_EXTERN_C_BEGIN

///
/// @brief Memory Allocation Descriptor (MAD), manages a single page.
///
typedef struct _ki_mad_t : public kfxx::rbtree_t<void *>::node_t {
	struct _ki_mad_t *next_free, *prev_free;

	size_t ref_count = 0;
	size_t pin_count = 0;

	ki_mm_rmlt_t *rmlt = nullptr;

	hal_spinlock_t spinlock = HAL_SPINLOCK_UNLOCKED;
} ki_mad_t;

typedef struct _ki_madpool_t ki_madpool_t;

typedef struct _ki_madpool_header_t {
	ki_madpool_t *prev, *next;
	size_t used_num;
} ki_madpool_header_t;

typedef struct _ki_madpool_t {
	ki_madpool_header_t header;
} ki_madpool_t;

static_assert(sizeof(ki_madpool_t) <= PAGESIZE);

///
/// @brief Physical Memory Region Descriptor
///
struct ki_pmad_t : public kfxx::rbtree_t<void *>::node_t {
	hal_spinlock_t spinlock = HAL_SPINLOCK_UNLOCKED;

	size_t len = 0;

	uint8_t type;
	bool is_initial_pmad = false;

	void *direct_map_base = nullptr;
	size_t direct_map_size = 0;

	// MAD pages are all preallocated at initializing stage.
	ki_mad_t *free_list = nullptr;

	kfxx::rbtree_t<void *> query_tree;

	size_t used_count = 0;

	ki_pmad_t();
};

extern ki_pmad_t ki_initial_pmad_storage[KI_INITIAL_MM_AREA_STORAGE_NUM];
extern kfxx::rbtree_t<void *> ki_pmad_tree;
extern size_t ki_pmad_number;

extern ki_madpool_t *ki_global_mad_pool_list;

// Initialized in the HAL.
extern size_t kh_mad_pool_descs_off, kh_mad_pool_descs_num_per_page;

extern size_t ki_num_available_phy_pages, ki_num_total_free_pages;
extern ps::semaphore_t ki_page_alloc_counter_semaphore;

#define KI_PMAD_FOREACH(i) \
	for (ki_pmad_t *i = static_cast<ki_pmad_t*>(ki_pmad_tree.begin().node); i; i = static_cast<ki_pmad_t*>(ki_pmad_tree.get_next(i, nullptr)))

ki_mad_t *kh_get_mad(void *pgaddr);

ki_pmad_t *ki_get_pmad(void *addr);

PBOS_EXTERN_C_END

#endif
