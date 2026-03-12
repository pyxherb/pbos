#ifndef _HAL_I386_MM_H_
#define _HAL_I386_MM_H_

#include <arch/i386/kargs.h>
#include <arch/i386/paging.h>
#include <pbos/km/assert.h>
#include <stdalign.h>
#include <string.h>
#include "mm/misc.h"
#include "mm/vmmgr/vm.h"

PBOS_EXTERN_C_BEGIN

#define HN_MAX_PGTAB_LEVEL 2

#define HN_VPM_LEVEL_MAX (HN_MAX_PGTAB_LEVEL - 1)

enum {
	HN_MM_INIT_STAGE_INITIAL = 0,
	HN_MM_INIT_STAGE_AREAS_INITIAL,
	HN_MM_INIT_STAGE_INITIAL_AREAS_INITED,
	HN_MM_INIT_STAGE_AREAS_INITED,
	HN_MM_INIT_STAGE_CUR_CONTEXT_LIST_INITED,
};

extern uint8_t hn_mm_init_stage;

typedef struct _mm_context_t mm_context_t;

PBOS_NODISCARD void *mm_vmalloc_early(
	mm_context_t *context,
	const void *minaddr,
	const void *maxaddr,
	size_t size,
	mm_pgaccess_t access);
PBOS_NODISCARD km_result_t hn_mm_mmap_early(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access);

void hn_mm_init();

PBOS_EXTERN_C_END

#endif
