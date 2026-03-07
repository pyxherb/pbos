#ifndef _HAL_X86_64_MM_MISC_H_
#define _HAL_X86_64_MM_MISC_H_

#include <arch/x86_64/seg.h>
#include <arch/x86_64/tss.h>
#include <pbos/mm/mm.h>

PBOS_EXTERN_C_BEGIN

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

extern size_t hn_tss_storage_num;
extern arch_tss_t *hn_tss_storage_ptr;
extern hn_kgdt_t *hn_gdt_storage_ptr;

uint8_t hn_to_kn_pmem_type(uint8_t memtype);

PBOS_EXTERN_C_END

#endif
