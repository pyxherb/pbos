#ifndef _HAL_X86_64_MM_HH_
#define _HAL_X86_64_MM_HH_

#include <arch/x86_64/mlayout.h>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/kh/mm/misc.hh>
#include "mm.h"
#include "mm/pgalloc/pgalloc.hh"

PBOS_EXTERN_C_BEGIN

typedef struct _hn_tmpmap_info_t {
	void *tmpmap_base;
	arch_pte_t *tmpmap_pgtab_base;
} hn_tmpmap_info_t;

typedef struct _mm_context_t {
	arch_pml4te_t *pml4t = nullptr;
} mm_context_t;

///
/// @brief Mapped kernel page table entry table.
///
extern arch_pte_t mm_kernel_initial_ptt[KINITPTT_SIZE / sizeof(arch_pte_t)];
extern arch_pde_t mm_kernel_initial_pdt[KINITPDT_SIZE / sizeof(arch_pde_t)];
extern arch_pdpte_t mm_kernel_initial_pdpt[KINITPDPT_SIZE / sizeof(arch_pdpte_t)];
extern arch_pml4te_t mm_kernel_initial_pml4t[KINITPML4T_SIZE / sizeof(arch_pml4te_t)];
static_assert(sizeof(mm_kernel_initial_pdt) == KINITPDT_SIZE);
static_assert(sizeof(mm_kernel_initial_pdpt) == KINITPDPT_SIZE);
static_assert(sizeof(mm_kernel_initial_pml4t) == KINITPML4T_SIZE);
extern void *mm_kernel_bottom_ptt_paddr;
extern void *mm_kernel_bottom_pdt_paddr;
extern void *mm_kernel_bottom_pdpt_paddr;
extern void *mm_kernel_initial_ptt_paddr;
extern void *mm_kernel_initial_pdt_paddr;
extern void *mm_kernel_initial_pdpt_paddr;
extern void *mm_kernel_initial_pml4t_paddr;

PBOS_EXTERN_C_END

#endif
