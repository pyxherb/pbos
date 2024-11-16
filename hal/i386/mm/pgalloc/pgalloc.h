#ifndef _HAL_I386_MM_PGALLOC_H_
#define _HAL_I386_MM_PGALLOC_H_

#include <arch/i386/kargs.h>
#include <arch/i386/misc.h>
#include <arch/i386/mlayout.h>
#include <arch/i386/paging.h>
#include <pbos/km/mm.h>
#include <stdint.h>

//
// MAD allocation types.
// For non-terminal MADs, only `MAD_ALLOC_FREE` and `MAD_ALLOC_KERNEL` (Used) are valid.
//
#define MAD_ALLOC_FREE 0x00		 // Free
#define MAD_ALLOC_KERNEL 0x01	 // Used by kernel
#define MAD_ALLOC_USER 0x02		 // Used by users
#define MAD_ALLOC_HARDWARE 0x03	 // Used by hardwares

enum {
	KN_PMEM_AVAILABLE = 0x00,  // Available for use
	KN_PMEM_CRITICAL,		   // Critical area, reserved
	KN_PMEM_HARDWARE,		   // Reserved for hardwares
	KN_PMEM_ACPI,			   // Reserved for ACPI
	KN_PMEM_HIBERNATION,	   // Reserved for hiberination
	KN_PMEM_PGTAB,			   // Page table
	KN_PMEM_BAD,			   // Unavailable
	KN_PMEM_END				   // End of the descriptor list.
};

#define MAD_P 0x01	// Present
#define MAD_S 0x02	// Shared
#define MAD_C 0x04	// Cached
#define MAD_M 0x08	// Mapped
#define MAD_L 0x10	// Locked
#define MAD_D 0x20	// Dirty
#define MAD_B 0x40	// Busy

///
/// @brief Memory Allocation Descriptor (MAD), manages a single ordered block.
///
typedef struct __packed _hn_mad_t {
	uint8_t flags : 8;
	uint8_t type : 4;
	uint32_t pgaddr : 20;
	uint32_t ref_count;
} hn_mad_t;

#define _mm_isvalidmad(mad) (((hn_mad_t *)mad)->flags & MAD_P)

//
// Order definitions.
// Greater order always with smaller block size.
//
#define MM_BLKSIZE(ord) (PAGESIZE << (ord))	 // Ordered block size
#define MM_BLKPGSIZE(ord) (1 << (ord))		 // Ordered paged block size
#define MM_PGWIND(ord, n) ((n) >> (ord))	 // Get ordered value, the value decreases with increment of the order
#define MM_PGUNWIND(ord, n) ((n) << (ord))	 // Get ordered value, the value increase with the order

#define ISINRANGE(min, size, n) ((((n) >= (min))) && (n < ((min) + (size))))

typedef struct __packed _hn_madpool_t {
	hn_mad_t descs[256];
	pgaddr_t next;
} hn_madpool_t;

static_assert(sizeof(hn_madpool_t) <= PAGESIZE);

///
/// @brief Physical Memory Region Descriptor
///
typedef struct __packed _hn_pmad_t {
	struct {
		pgaddr_t base : 20;	 // Paged base address
		pgsize_t len : 20;	 // Length in pages
		uint8_t type : 5;	 // Type
		uint8_t maxord : 3;	 // Maximum order
	} attribs;
	// MAD pages were all preallocated at initializing stage.
	pgaddr_t madpools[MM_MAXORD + 1];
} hn_pmad_t;

extern hn_pmad_t hn_pmad_list[ARCH_MMAP_MAX + 1];

#define PMAD_FOREACH(i) \
	for (hn_pmad_t *i = hn_pmad_list; i->attribs.type != KN_PMEM_END; ++i)

hn_pmad_t *hn_pmad_get(pgaddr_t addr);
hn_mad_t *hn_find_mad(hn_madpool_t *pool, uint32_t pgaddr, uint8_t order);

pgaddr_t hn_alloc_freeblk_in_area(hn_pmad_t *area, uint8_t order);
pgaddr_t hn_alloc_freeblk(uint8_t type, uint8_t order);

void hn_set_pgblk_used(pgaddr_t pgaddr, uint8_t type, uint8_t order);
void hn_set_pgblk_free(pgaddr_t addr, uint8_t order);

void hn_madpool_init(hn_madpool_t *madpool, pgaddr_t last, pgaddr_t next);

#endif
