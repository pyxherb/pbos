#ifndef _HAL_I386_MM_MISC_H_
#define _HAL_I386_MM_MISC_H_

#include <hal/i386/mm.h>
#include <oicos/km/mm.h>
#include <arch/i386/seg.h>
#include "pgalloc/pgalloc.h"

#define SELECTOR_KCODE SELECTOR(0, 0, 1)
#define SELECTOR_KDATA SELECTOR(0, 0, 2)
#define SELECTOR_UCODE SELECTOR(0, 0, 3)
#define SELECTOR_UDATA SELECTOR(0, 0, 4)

typedef struct __packed _hn_kgdt_t {
	arch_gdt_desc_t null_desc;
	arch_gdt_desc_t kcode_desc;
	arch_gdt_desc_t kdata_desc;
	arch_gdt_desc_t ucode_desc;
	arch_gdt_desc_t udata_desc;
} hn_kgdt_t;

#define hn_kernel_pdt ((arch_pde_t *)KPDT_VBASE)
#define hn_kernel_pgt ((arch_pte_t *)KPGT_VBASE)
#define hn_bottom_pgt ((arch_pte_t *)KBOTTOMPGT_VBASE)

extern hn_kgdt_t _kgdt;

uint8_t hn_to_kn_pmem_type(uint8_t memtype);

__always_inline inline uint8_t hn_get_alloc_order(size_t size) {
	// Order of block to allocate.
	uint16_t order = 0;

	// Get minimum order of block which can contain the whole fragment.
	while (size > MM_BLKSIZE(order))
		order++;
	order = MIN(order, MM_MAXORD);	// Cannot exceed the maximum order.

	return order;
}

#endif
