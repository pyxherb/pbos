#ifndef _HAL_I386_MM_VM_H_
#define _HAL_I386_MM_VM_H_

#include <hal/i386/mm.h>

/// @brief Allocate a single virtual page in kernel space.
/// @param pgdir Page directory to be operated.
/// @return Paged address of allocated virtual page, NULLPG otherwise.
pgaddr_t hn_kvpgalloc(const arch_pde_t *pgdir);

/// @brief Allocate a single virtual page from specified range.
/// @param pgdir Page directory to be operated.
/// @param minaddr Minimum address of the range.
/// @param maxaddr Maximum address of the range.
/// @return Paged address of allocated virtual page, NULLPG otherwise.
pgaddr_t hn_vpgalloc(const arch_pde_t *pgdir, pgaddr_t minaddr, pgaddr_t maxaddr);

/// @brief Map physical pages to a virtual memory area.
/// @param pdt Target PDT.
/// @param paddr Address to a continuous physical area to be mapped.
/// @param vaddr Address to a virtual memory area to map.
/// @param pg_num Number of pages to map.
/// @param mask PTE mask.
/// @return Result code of the mapping operation.
/// @note Mapped page will not be referenced!
km_result_t hn_pgmap(arch_pde_t *pdt, pgaddr_t paddr, pgaddr_t vaddr, pgsize_t pg_num, uint16_t mask);

#define hn_unpgmap(pdt, vaddr, pg_num) hn_pgmap((pdt), (PGADDR_MAX) - (pg_num), (vaddr), (pg_num), ~PTE_P)

void *hn_getmap(const arch_pde_t *pgdir, const void *vaddr);

/// @brief Allocate a page table for a memory context.
/// @param ctxt Context to be operated.
/// @param pdx PDX of the page table to be allocated.
/// @return 
pgaddr_t hn_mmctxt_pgtaballoc(mm_context_t *ctxt, uint16_t pdx);

/// @brief Free a page table for a memory context.
/// @param ctxt Context to be operated.
/// @param pdx PDX of the page table to be freed.
void hn_mmctxt_pgtabfree(mm_context_t *ctxt, uint16_t pdx);

/// @brief Map a physical page temporarily.
/// @param pgpaddr Paged address of the physical page.
/// @param pg_num Number of pages
/// @param mask PTE mask for the pages.
/// @return Paged virtual address to the first mapped page, NULLPG if failed.
pgaddr_t hn_tmpmap(pgaddr_t pgpaddr, pgsize_t pg_num, uint16_t mask);

/// @brief Map a temporary-mapped page previously
/// @param addr Paged virtual address to the first mapped page.
void hn_tmpunmap(pgaddr_t addr);

#endif
