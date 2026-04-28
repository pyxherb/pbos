#ifndef _HAL_X86_64_MM_H_
#define _HAL_X86_64_MM_H_

#include <arch/x86_64/kargs.h>
#include <arch/x86_64/paging.h>
#include <pbos/km/assert.h>
#include <stdalign.h>
#include <string.h>
#include "mm/misc.h"
#include "mm/vmmgr/vm.h"

PBOS_EXTERN_C_BEGIN

#define HN_MAX_PGTAB_LEVEL 4

#define HN_VPM_LEVEL_MAX (HN_MAX_PGTAB_LEVEL - 1)

enum {
	HN_MM_INIT_STAGE_INITIAL = 0,
	HN_MM_INIT_STAGE_AREAS_INITIAL,
	HN_MM_INIT_STAGE_INITIAL_AREAS_INITED,
	HN_MM_INIT_STAGE_AREAS_INITED,
};

extern uint8_t hn_mm_init_stage;

extern void *mm_kernel_bottom_mapping_base_vaddr;
extern char mm_kernel_initial_stack[1024 * 96];

typedef struct _mm_context_t mm_context_t;

typedef struct _hn_tmpmap_info_t hn_tmpmap_info_t;
extern hn_tmpmap_info_t *hn_tmpmap_info_storage;

PBOS_NODISCARD void *mm_vmalloc_early(
	mm_context_t *context,
	const void *minaddr,
	const void *maxaddr,
	size_t size,
	mm_pgaccess_t access);
PBOS_NODISCARD void *mm_kvmalloc_early(mm_context_t *context, size_t size, mm_pgaccess_t access);
///
/// @brief Map a page-sized region of memory in the early stage.
///
/// @param context Context for mapping.
/// @param vaddr Virtual address to be mapped.
/// @param paddr Corresponding physical address.
/// @param access Access modifier of the region.
/// @param pdpte_paddr Reserved PDP physical address for mapping if the PDP does not exist.
/// @param pde_paddr Reserved page directory physical address for mapping if the page directory does not exist.
/// @param pte_paddr Reserved page table physical address for mapping if the page table does not exist.
/// @return Bit mask of reserved pages used state. 0b001 for pte_paddr used, 0b010 for pde_paddr used, 0b100 for pdpte_padr used.
///
PBOS_NODISCARD uint8_t hn_mm_mmap_early(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	mm_pgaccess_t access,
	void *pdpte_paddr,
	void *pde_paddr,
	void *pte_paddr);

void hn_mm_init();

PBOS_EXTERN_C_END

#endif
