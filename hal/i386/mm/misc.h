#ifndef _HAL_I386_MM_MISC_H_
#define _HAL_I386_MM_MISC_H_

#include <hal/i386/mm.h>
#include <pbos/km/mm.h>
#include <arch/i386/seg.h>
#include <arch/i386/tss.h>
#include "pgalloc/pgalloc.h"

#define SELECTOR_KCODE SELECTOR(0, 0, 1)
#define SELECTOR_KDATA SELECTOR(0, 0, 2)
#define SELECTOR_UCODE SELECTOR(3, 0, 3)
#define SELECTOR_UDATA SELECTOR(3, 0, 4)
#define SELECTOR_TSS SELECTOR(0, 0, 5)

typedef struct PB_PACKED _hn_kgdt_t {
	arch_gdt_desc_t null_desc;
	arch_gdt_desc_t kcode_desc;
	arch_gdt_desc_t kdata_desc;
	arch_gdt_desc_t ucode_desc;
	arch_gdt_desc_t udata_desc;
	arch_gdt_desc_t tss_desc;
} hn_kgdt_t;

#define hn_kernel_pdt ((arch_pde_t *)KPDT_VBASE)
#define hn_kernel_pgt ((arch_pte_t *)KPGT_VBASE)
#define hn_bottom_pgt ((arch_pte_t *)KBOTTOMPGT_VBASE)

extern hn_kgdt_t hn_kgdt;

extern size_t hn_tss_storage_num;
extern arch_tss_t *hn_tss_storage_ptr;

extern char **hn_tss_stacks;

uint8_t hn_to_kn_pmem_type(uint8_t memtype);

#endif
