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

enum {
	KN_PMEM_END = 0x00,	  // End of the descriptor list.
	KN_PMEM_AVAILABLE,	  // Available for use
	KN_PMEM_BOOTDATA,	  // Boot data
	KN_PMEM_CRITICAL,	  // Critical area, reserved
	KN_PMEM_HARDWARE,	  // Reserved for hardwares
	KN_PMEM_ACPI,		  // Reserved for ACPI
	KN_PMEM_HIBERNATION,  // Reserved for hiberination
	KN_PMEM_PGTAB,		  // Page table
	KN_PMEM_BAD,		  // Unavailable
};

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
typedef struct _hn_mad_t : public kfxx::rbtree_t<pgaddr_t>::node_t {
	struct _hn_mad_t *next_free, *prev_free;

	uint32_t ref_count;

	uint8_t flags : 8;
	uint8_t type : 8;
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
typedef struct _hn_pmad_t {
	struct {
		pgaddr_t base;	// Paged base address
		pgsize_t len;	// Length in pages
		uint8_t type;	// Type
	} attribs;
	void *direct_map_base;
	size_t direct_map_size;
	// MAD pages were all preallocated at initializing stage.
	hn_mad_t *free_list;
	kfxx::rbtree_t<pgaddr_t> query_tree;
} hn_pmad_t;

extern hn_pmad_t hn_pmad_list[ARCH_MMAP_MAX + 1];
extern size_t hn_pmad_number;

extern hn_madpool_t *hn_global_mad_pool_list;

#define PMAD_FOREACH(i) \
	for (hn_pmad_t *i = hn_pmad_list; i->attribs.type != KN_PMEM_END; ++i)

hn_mad_t *hn_get_mad(pgaddr_t pgaddr);

hn_pmad_t *hn_pmad_get(pgaddr_t addr);

pgaddr_t hn_alloc_freeblk_in_area(hn_pmad_t *area);
pgaddr_t hn_alloc_freeblk(uint8_t type);

void hn_set_pgblk_used(pgaddr_t pgaddr, uint8_t type);
void hn_set_pgblk_free(pgaddr_t addr);

PBOS_EXTERN_C_END

#endif
