#include <pbos/kh/mm/misc.hh>
#include <pbos/ki/km/proc.hh>
#include <pbos/ki/km/symbol.hh>

mm_context_t **mm_cur_contexts = nullptr;

km_result_t mm_mmap(mm_context_t *ctxt,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mmap_flags_t flags) {
	return kh_mmap(ctxt, vaddr, paddr, size, access, flags);
}
KI_EXPORT_IMAGE_SYMBOL(mm_mmap);

void mm_unmmap(mm_context_t *ctxt, void *vaddr, size_t size, mmap_flags_t flags) {
	return kh_unmmap(ctxt, vaddr, size, flags);
}
KI_EXPORT_IMAGE_SYMBOL(mm_unmmap);

void mm_set_page_access(
	mm_context_t *context,
	const void *vaddr,
	size_t size,
	mm_pgaccess_t access) {
	kh_set_page_access(context, vaddr, size, access);
}
KI_EXPORT_IMAGE_SYMBOL(mm_set_page_access);

void *mm_vmalloc(mm_context_t *context,
	const void *minaddr,
	const void *maxaddr,
	size_t size,
	mm_pgaccess_t access,
	mm_vmalloc_flags_t flags) {
	return kh_vmalloc(context, minaddr, maxaddr, size, access, flags);
}
KI_EXPORT_IMAGE_SYMBOL(mm_vmalloc);

void *mm_kvmalloc(mm_context_t *ctxt, size_t size, mm_pgaccess_t access, mm_vmalloc_flags_t flags) {
	return kh_kvmalloc(ctxt, size, access, flags);
}
KI_EXPORT_IMAGE_SYMBOL(mm_kvmalloc);

void *mm_getmap(mm_context_t *ctxt, const void *vaddr, mm_pgaccess_t *pgaccess_out) {
	return kh_getmap(ctxt, vaddr, pgaccess_out);
}
KI_EXPORT_IMAGE_SYMBOL(mm_getmap);
