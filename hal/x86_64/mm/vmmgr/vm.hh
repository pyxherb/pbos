#ifndef _HAL_X86_64_MM_VM_HH_
#define _HAL_X86_64_MM_VM_HH_

#include <hal/x86_64/mm.hh>
#include <arch/x86_64/paging.h>
#include <pbos/km/result.h>

PBOS_EXTERN_C_BEGIN

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

typedef km_result_t (*hn_pgtab_walker)(arch_pde_t *pde, arch_pte_t *pte, uint16_t pdx, uint16_t ptx, void *exargs);

void *hn_getmap(const arch_pde_t *pgdir, const void *vaddr, uint16_t *mask_out);

/// @brief Map physical pages temporarily.
/// @param pgpaddr Paged address of the physical page.
/// @param pg_num Number of pages
/// @param mask PTE mask for the pages.
/// @return Paged virtual address to the first mapped page, NULLPG if failed.
PBOS_NODISCARD void *hn_tmpmap_early(void *paddr, size_t size, uint16_t mask);

/// @brief Map a temporary-mapped page previously
/// @param addr Paged virtual address to the first mapped page.
void hn_tmpunmap_early(void *vaddr, size_t size);

PBOS_NODISCARD void *hn_tmpmap_post(void *paddr, size_t size, uint16_t mask);

void hn_tmpunmap_post(void *vaddr, size_t size);

void *hn_get_pgtab_paddr(mm_context_t *ctxt, const void *vaddr, mm_pgaccess_t *pgaccess_out);

PBOS_EXTERN_C_END

#endif
