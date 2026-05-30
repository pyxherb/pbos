#ifndef _PBOS_KH_MM_MISC_H_
#define _PBOS_KH_MM_MISC_H_

#include <pbos/ki/mm/context.hh>
#include <pbos/kf/misc.h>

PBOS_EXTERN_C_BEGIN

PBOS_PURE size_t kh_get_page_size();

void *kh_get_direct_mmap(void *paddr);

PBOS_NODISCARD km_result_t kh_mmap(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_page_access_t access,
	mmap_flags_t flags);

void kh_munmap(mm_context_t *ctxt, void *vaddr, size_t size, mmap_flags_t flags);

typedef kf_control_flow_t (*kh_pgtab_walker)(void *vaddr, void *paddr, mm_page_access_t page_access, void *user_data);

enum {
	KH_WALK_PGTAB_SKIP_UNMAPPED = 1
};

typedef uint32_t kh_walk_pgtab_flags_t;

void kh_walk_pgtab(mm_context_t *ctxt, void *vaddr, size_t size, kh_pgtab_walker walker, void *user_data, kh_walk_pgtab_flags_t flags);

void kh_set_page_access(
	mm_context_t *context,
	const void *vaddr,
	size_t size,
	mm_page_access_t access);

void *kh_vmalloc(mm_context_t *context,
	const void *minaddr,
	const void *maxaddr,
	size_t size,
	mm_page_access_t access,
	mm_vmalloc_flags_t flags);

void *kh_kvmalloc(mm_context_t *ctxt, size_t size, mm_page_access_t access, mm_vmalloc_flags_t flags);

void *kh_getmap(mm_context_t *ctxt, const void *vaddr, mm_page_access_t *page_access_out);

PBOS_NODISCARD km_result_t kh_mm_alloc_context(mm_context_t *cur_context, mm_context_t **new_context_out);;

void ki_mm_lock_vmr(mm_context_t *mm_context);

void ki_mm_unlock_vmr(mm_context_t *mm_context);

void ki_mm_lock_area(mm_context_t *mm_context, mm_vmr_t *vmr);

void ki_mm_unlock_area(mm_context_t *mm_context, mm_vmr_t *vmr);

PBOS_EXTERN_C_END

#endif
