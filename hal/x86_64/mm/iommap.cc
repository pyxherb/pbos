#include "iommap.hh"
#include <pbos/km/logger.h>
#include <hal/x86_64/mm.hh>
#include <pbos/kfxx/scope_guard.hh>

PBOS_EXTERN_C_BEGIN

km_result_t mm_iommap(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mm_iommap_flags_t flags) {
	km_result_t result;
	mmap_flags_t mmap_flags = MMAP_NO_RC;

	if ((result = mm_mmap(context, vaddr, paddr, size, access, mmap_flags)))
		return result;

	return KM_RESULT_OK;
}

void mm_uniommap(mm_context_t *context, void *vaddr, size_t size, mm_iommap_flags_t flags) {
	km_result_t result;
	mmap_flags_t mmap_flags = MMAP_NO_RC;

	mm_unmmap(context, vaddr, size, mmap_flags);
}

PBOS_EXTERN_C_END
