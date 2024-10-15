#ifndef _HAL_I386_MM_H_
#define _HAL_I386_MM_H_

#include <arch/i386/kargs.h>
#include <arch/i386/paging.h>
#include <pbos/km/assert.h>
#include <pbos/km/mm.h>
#include <stdalign.h>
#include <string.h>
#include "mm/misc.h"
#include "mm/skid.h"
#include "mm/vmmgr/vm.h"

typedef struct _mm_context_t {
	arch_pde_t *pdt;
} mm_context_t;

void hn_mm_init();

#endif
