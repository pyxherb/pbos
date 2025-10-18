#include "iommap.hh"
#include <hal/i386/mm.h>
#include <math.h>
#include <pbos/km/logger.h>

PBOS_EXTERN_C_BEGIN

hn_madpool_t *hn_global_mad_pool_list = NULL;

km_result_t mm_iommap(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mm_iommap_flags_t flags) {

}

void mm_uniommap(mm_context_t *context, void *vaddr, size_t size, mm_iommap_flags_t flags) {

}

PBOS_EXTERN_C_END
