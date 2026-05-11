#ifndef _HAL_X86_64_MM_PGALLOC_H_
#define _HAL_X86_64_MM_PGALLOC_H_

#include <arch/x86_64/kargs.h>
#include <arch/x86_64/misc.h>
#include <arch/x86_64/mlayout.h>
#include <arch/x86_64/paging.h>
#include <stdint.h>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/ki/mm/misc.hh>
#include <pbos/hal/spinlock.h>

PBOS_EXTERN_C_BEGIN

///
/// @brief Memory Allocation Descriptor (MAD), manages a single page.
///
typedef struct _hn_mad_t : public kfxx::rbtree_t<void *>::node_t {
	struct _hn_mad_t *next_free, *prev_free;

	hal_spinlock_t spinlock;

	size_t ref_count;
} hn_mad_t;

typedef struct _hn_madpool_t hn_madpool_t;

typedef struct _hn_madpool_header_t {
	hn_madpool_t *prev, *next;
	size_t used_num;
} hn_madpool_header_t;

typedef struct _hn_madpool_t {
	hn_madpool_header_t header;
	hn_mad_t descs[(PAGESIZE - sizeof(hn_madpool_header_t)) / sizeof(hn_mad_t)];
} hn_madpool_t;

static_assert(sizeof(hn_madpool_t) <= PAGESIZE);

///
/// @brief Physical Memory Region Descriptor
///
struct hn_pmad_t {
	void *base;
	size_t len;
	uint8_t type;
	void *direct_map_base;
	size_t direct_map_size;
	// MAD pages are all preallocated at initializing stage.
	hn_mad_t *free_list = nullptr;
	kfxx::rbtree_t<void *> query_tree;
};

extern hn_pmad_t hn_pmad_list[ARCH_MMAP_MAX + 1];
extern size_t hn_pmad_number;

extern hn_madpool_t *hn_global_mad_pool_list;

#define PMAD_FOREACH(i) \
	for (hn_pmad_t *i = hn_pmad_list; i - hn_pmad_list < hn_pmad_number; ++i)

hn_mad_t *hn_get_mad(void *pgaddr);

hn_pmad_t *hn_pmad_get(void *addr);

void *hn_alloc_freeblk_in_area(hn_pmad_t *area);
void *hn_alloc_freeblk(uint8_t type);

void hn_set_pgblk_used(void *addr);
void hn_set_pgblk_free(void *addr);

PBOS_EXTERN_C_END

#endif
