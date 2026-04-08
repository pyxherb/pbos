#ifndef _PBOS_KH_MM_H_
#define _PBOS_KH_MM_H_

#include <pbos/kn/mm/mm.hh>

PBOS_EXTERN_C_BEGIN

PBOS_NODISCARD km_result_t kh_mmap(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mmap_flags_t flags);

void kh_unmmap(mm_context_t *ctxt, void *vaddr, size_t size, mmap_flags_t flags);

void kh_set_page_access(
	mm_context_t *context,
	const void *vaddr,
	size_t size,
	mm_pgaccess_t access);

void *kh_vmalloc(mm_context_t *context,
	const void *minaddr,
	const void *maxaddr,
	size_t size,
	mm_pgaccess_t access,
	mm_vmalloc_flags_t flags);

void *kh_kvmalloc(mm_context_t *ctxt, size_t size, mm_pgaccess_t access, mm_vmalloc_flags_t flags);

void *kh_getmap(mm_context_t *ctxt, const void *vaddr, mm_pgaccess_t *pgaccess_out);

PBOS_EXTERN_C_END

#endif
