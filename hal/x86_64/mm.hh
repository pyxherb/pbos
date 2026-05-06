#ifndef _HAL_X86_64_MM_HH_
#define _HAL_X86_64_MM_HH_

#include <arch/x86_64/mlayout.h>
#include <arch/x86_64/tss.h>
#include <arch/x86_64/seg.h>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/kh/mm/misc.hh>
#include "mm/pgalloc/pgalloc.hh"

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
extern hn_tmpmap_info_t *hn_tmpmap_storage_ptr;

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

#define SELECTOR_KCODE SELECTOR(0, 0, 1)
#define SELECTOR_KDATA SELECTOR(0, 0, 2)
#define SELECTOR_UCODE SELECTOR(3, 0, 3)
#define SELECTOR_UDATA SELECTOR(3, 0, 4)
#define SELECTOR_TSS SELECTOR(0, 0, 5)

typedef struct PBOS_PACKED _hn_kgdt_t {
	arch_gdt_desc_t null_desc;
	arch_gdt_desc_t kcode_desc;
	arch_gdt_desc_t kdata_desc;
	arch_gdt_desc_t ucode_desc;
	arch_gdt_desc_t udata_desc;
	arch_gdt_desc_t tss_desc1;
	arch_gdt_desc_t tss_desc2;
} hn_kgdt_t;

extern hn_kgdt_t hn_init_kgdt;

extern arch_tss_t *hn_tss_storage_ptr;
extern hn_kgdt_t *hn_gdt_storage_ptr;

uint8_t hn_to_ki_pmem_type(uint8_t memtype);

PBOS_EXTERN_C_END

#endif
