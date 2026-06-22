#ifndef _ARCH_X86_64_MEMCONF_H_
#define _ARCH_X86_64_MEMCONF_H_

#include <pbos/ki/mm/memconf.h>
#include "mlayout.h"

PBOS_EXTERN_C_BEGIN

enum {
	PAGESIZE = 4096
};

enum {
	PML4X_MAX = 0x1ff,
	PML4X_MIN = 0,

	PDPTX_MAX = 0x1ff,
	PDPTX_MIN = 0,

	PDX_MAX = 0x1ff,
	PDX_MIN = 0,

	PTX_MAX = 0x1ff,
	PTX_MIN = 0,

	PGOFF_MAX = PAGESIZE - 1,
	PGOFF_MIN = 0,
};

typedef uint64_t pgaddr_t;
typedef uint64_t pgsize_t;

enum {
	PGADDR_MAX = 0xfffff,
	PGADDR_MIN = 0,
};

/// @brief Round up a linear address into page-aligned.
#define PGCEIL(addr) ((uintptr_t)((((size_t)(addr)) + (PAGESIZE - 1)) & (~(PAGESIZE - 1))))
/// @brief Round down a linear address into page-aligned.
#define PGFLOOR(addr) ((uintptr_t)(((size_t)(addr)) & (~(PAGESIZE - 1))))

/// @brief Round up a linear address into a paged address.
#define PGROUNDUP(addr) \
	(((pgaddr_t)(addr) + PGOFF_MAX) >> 12)
/// @brief Round down a linear address into a paged address
#define PGROUNDDOWN(addr) ((((pgaddr_t)(addr))) >> 12)

enum {
	PML4_SHIFT = 39,
	PDPT_SHIFT = 30,
	PD_SHIFT = 21,
	PT_SHIFT = 12,
};

#define PML4X(va) (((uintptr_t)(va) >> PML4_SHIFT) & 0x1ffULL)
#define PDPTX(va) (((uintptr_t)(va) >> PDPT_SHIFT) & 0x1ffULL)
#define PDX(va) (((uintptr_t)(va) >> PD_SHIFT) & 0x1ffULL)
#define PTX(va) (((uintptr_t)(va) >> PT_SHIFT) & 0x1ffULL)
#define PGOFF(addr) (((uint64_t)(addr)) & 0xfffULL)

#define ADDR_PREFIX(addr) (((uintptr_t)(addr)) & 0xffff000000000000ULL)
#define ADDR_VALUE(addr) (((uintptr_t)(addr)) & ~0xffff000000000000ULL)

#define ISVALIDPG(pg) \
	((pg) != 0)	 // Check if a page address is valid

/// @brief Get corresponding kernel virtual address.
#define KVADDR(pml4x, pdptx, pdx, ptx, offset) ((void *)((0xffff000000000000ULL) | ((((uint64_t)pml4x) << PML4_SHIFT) | (((uint64_t)pdptx) << PDPT_SHIFT) | ((uint64_t)pdx) << PD_SHIFT) | (((uint64_t)ptx) << PT_SHIFT) | (offset)))
/// @brief Get corresponding user virtual address.
#define UVADDR(pml4x, pdptx, pdx, ptx, offset) ((void *)(((((uint64_t)pml4x) << PML4_SHIFT) | (((uint64_t)pdptx) << PDPT_SHIFT) | ((uint64_t)pdx) << PD_SHIFT) | (((uint64_t)ptx) << PT_SHIFT) | (offset)))
#define VADDR(prefix, pml4x, pdptx, pdx, ptx, offset) ((void *)((prefix) | ((((uint64_t)pml4x) << PML4_SHIFT) | (((uint64_t)pdptx) << PDPT_SHIFT) | ((uint64_t)pdx) << PD_SHIFT) | (((uint64_t)ptx) << PT_SHIFT) | (offset)))
/// @brief Get unpaged linear address from paged address.
#define UNPGADDR(addr) ((void *)((addr) << 12))
#define UNPGSIZE(size) ((size_t)((size) << 12))

enum {
	KVADDR_PREFIX = 0xffff000000000000ULL
};

//
// Paging configurations.
//
extern const ki_paging_config_t KI_PAGING_CONFIG_48BIT;

PBOS_EXTERN_C_END

#endif
