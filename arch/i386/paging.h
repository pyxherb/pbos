#ifndef _ARCH_I386_PAGING_H_
#define _ARCH_I386_PAGING_H_

#include <pbos/common.h>
#include <stdint.h>

#define PAGESIZE 4096

#define PGADDR_MAX 0xfffff
#define PGADDR_MIN 0

#define PDX_MAX 0x3ff
#define PDX_MIN 0

#define PTX_MAX 0x3ff
#define PTX_MIN 0

#define PGOFF_MAX ((PAGESIZE) - 1)
#define PGOFF_MIN 0

#define PAGESIZE_MAX 0x100000
#define PAGESIZE_MIN 0

#define NULLPG (PGADDR_MAX + 1)	 // Used for representing an invalid page

#define ISVALIDPG(pg) \
	(((pg) != 0) && ((pg) <= PGADDR_MAX))  // Check if a page address is valid

typedef uint32_t pgaddr_t;
typedef uint32_t pgsize_t;

/// @brief Get index in corresponding PDE.
#define PDX(addr) ((((uint32_t)(addr)) >> 22) & 0x3ff)
#define PGDX(addr) ((((uint32_t)(addr)) >> 10) & 0x3ff)
/// @brief Get index in corresponding PTE.
#define PTX(addr) ((((uint32_t)(addr)) >> 12) & 0x3ff)
#define PGTX(addr) (((uint32_t)(addr)) & 0x3ff)
/// @brief Get offset in corresponding page frame.
#define PGOFF(addr) (((uint32_t)(addr)) & 0xfff)

/// @brief Get corresponding virtual address.
#define VADDR(pdx, ptx, offset) ((void *)((pdx) << 22 | (ptx) << 12 | (offset)))
/// @brief Get linear paged address.
#define PGADDR(pdx, ptx) ((pgaddr_t)((pdx) << 10 | (ptx)))
/// @brief Get unpaged linear address from paged address.
#define UNPGADDR(addr) ((void *)((addr) << 12))
#define UNPGSIZE(size) ((size_t)((size) << 12))

/// @brief Round up a linear address into a paged address.
#define PGROUNDUP(addr) \
	(((pgaddr_t)(addr) >> 12) + ((((pgaddr_t)(addr)) & PGOFF_MAX) ? 1 : 0))
/// @brief Round down a linear address into a paged address
#define PGROUNDDOWN(addr) ((((pgaddr_t)(addr))) >> 12)
/// @brief Round up a linear address into page-aligned.
#define PGCEIL(addr) ((uintptr_t)((((size_t)(addr)) + PGOFF_MAX) & (~PGOFF_MAX)))
/// @brief Round down a linear address into page-aligned.
#define PGFLOOR(addr) ((uintptr_t)(((size_t)(addr)) & (~PGOFF_MAX)))

#define PDE_P 0x0001   // Present
#define PDE_RW 0x0002  // Read/Write
#define PDE_U 0x0004   // User
#define PDE_WT 0x0008  // Write-Through
#define PDE_CD 0x0010  // Cache Disabled
#define PDE_A 0x0020   // Accessed
#define PDE_4M 0x0080  // Enable 4MB paging

typedef struct PB_PACKED _arch_pde_t {
	uint16_t mask : 12;
	pgaddr_t address : 20;
} arch_pde_t;

#define PTE_P 0x0001	   // Present
#define PTE_RW 0x0002	   // Read/Write
#define PTE_U 0x0004	   // User
#define PTE_WT 0x0008	   // Write-Through
#define PTE_CD 0x0010	   // Cache Disabled
#define PTE_A 0x0020	   // Accessed
#define PTE_D 0x0040	   // Dirty
#define PTE_AT 0x0080	   // Page Attribute Table
#define PTE_G 0x0080	   // Global
#define PTE_AVAIL0 0x0100  // Available bit 0
#define PTE_AVAIL1 0x0200  // Available bit 1
#define PTE_XD 0x0400	   // Execute Disabled

typedef struct PB_PACKED _arch_pte_t {
	uint16_t mask : 12;
	pgaddr_t address : 20;
} arch_pte_t;

#define arch_invlpg(addr)                                      \
	__asm__ __volatile__("invlpg (%0)" ::"b"((uint32_t)(addr)) \
						 : "memory")
#define arch_lpdt(paddr) \
	arch_wcr3((arch_rcr3() & ~0xfffff000) | (((uint32_t)(paddr)) << 12))
#define arch_spdt() (arch_rcr3() & 0xfffff000)

#endif
