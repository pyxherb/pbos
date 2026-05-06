#ifndef _HAL_X86_64_MM_PGALLOC_H_
#define _HAL_X86_64_MM_PGALLOC_H_

#include <arch/x86_64/kargs.h>
#include <arch/x86_64/misc.h>
#include <arch/x86_64/mlayout.h>
#include <arch/x86_64/paging.h>
#include <stdint.h>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/kn/mm/mm.hh>

PBOS_EXTERN_C_BEGIN

//
// MAD allocation types.
// For non-terminal MADs, only `MAD_ALLOC_FREE` and `MAD_ALLOC_KERNEL` (Used) are valid.
//
#define MAD_ALLOC_FREE 0x00		 // Free
#define MAD_ALLOC_KERNEL 0x01	 // Used by kernel
#define MAD_ALLOC_USER 0x02		 // Used by users
#define MAD_ALLOC_HARDWARE 0x03	 // Used by hardwares

#define MAD_P 0x01	// Present
#define MAD_S 0x02	// Shared
#define MAD_C 0x04	// Cached
#define MAD_M 0x08	// Mapped
#define MAD_L 0x10	// Locked
#define MAD_D 0x20	// Dirty
#define MAD_B 0x40	// Busy

///
/// @brief Memory Allocation Descriptor (MAD), manages a single page.
///
typedef struct _hn_mad_t : public kfxx::rbtree_t<void *>::node_t {
	struct _hn_mad_t *next_free, *prev_free;

	size_t ref_count;
} hn_mad_t;

#define ISINRANGE(min, size, n) ((((n) >= (min))) && (n < ((min) + (size))))

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
	// MAD pages were all preallocated at initializing stage.
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
