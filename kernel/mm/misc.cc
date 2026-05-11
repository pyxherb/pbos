#include <pbos/km/logger.h>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/kh/mm/misc.hh>
#include <pbos/ki/km/proc.hh>
#include <pbos/ki/km/symbol.hh>

mm_context_t **mm_cur_contexts = nullptr;

_ps_vmr_t::_ps_vmr_t() {
}

km_result_t mm_mmap(mm_context_t *ctxt,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mmap_flags_t flags) {
	if (!ctxt)
		return KM_RESULT_INVALID_ARGS;
	if (!size)
		return KM_RESULT_INVALID_ARGS;

	// TODO: Check if the vaddr will overflow...
	void *aligned_vaddr = (void *)PGFLOOR(vaddr), *aligned_paddr = (void *)PGFLOOR(paddr);
	size_t aligned_size = PGCEIL(size);
	char *vaddr_limit = (char *)aligned_vaddr + (size - 1);
	bool is_user_space = mm_is_user_space(aligned_vaddr);

	if (is_user_space !=
		mm_is_user_space(vaddr_limit))
		return KM_RESULT_INVALID_ARGS;

	kfxx::scope_guard remove_vmr_guard([ctxt, aligned_vaddr]() noexcept {
		if (auto node = ctxt->vmr_tree.find(aligned_vaddr))
			ctxt->vmr_tree.remove(node);
	});

	if (!(flags & MMAP_IGNORE_VMR)) {
		if (is_user_space) {
			ps_vmr_t *vmr_begin, *vmr_end;

			vmr_begin = static_cast<ps_vmr_t *>(ctxt->vmr_tree.find_max_lteq(aligned_vaddr));
			vmr_end = static_cast<ps_vmr_t *>(ctxt->vmr_tree.find_max_lteq(vaddr_limit));

			if (ctxt->vmr_tree.size()) {
				if ((vmr_end) || ((vmr_begin) && (((char *)vmr_begin->rb_value) + vmr_begin->size >= vaddr)))
					return KM_RESULT_EXISTED;
			}

			ps_vmr_t *new_vmr = (ps_vmr_t *)kima_alloc(&ctxt->kima_vmr_pool, sizeof(ps_vmr_t), alignof(ps_vmr_t));
			if (!new_vmr)
				return KM_RESULT_NO_MEM;
			kfxx::construct_at<ps_vmr_t>(new_vmr);

			new_vmr->rb_value = vaddr;
			new_vmr->size = size;
			new_vmr->access = access;

			ctxt->vmr_tree.insert_unwrap(new_vmr);
		}
	}

	km_result_t result;

	if (KM_FAILED(result = kh_mmap(ctxt, aligned_vaddr, aligned_paddr, aligned_size, access, flags)))
		return result;

	remove_vmr_guard.release();

	return KM_RESULT_OK;
}
KI_EXPORT_IMAGE_SYMBOL(mm_mmap);

km_result_t mm_unmmap(mm_context_t *ctxt, void *vaddr, size_t size, mmap_flags_t flags) {
	// TODO: Check if the vaddr will overflow...
	if (!ctxt)
		return KM_RESULT_INVALID_ARGS;
	if (!size)
		return KM_RESULT_INVALID_ARGS;
	void *aligned_vaddr = (void *)PGFLOOR(vaddr);
	size_t aligned_size = PGCEIL(size);
	char *vaddr_limit = (char *)aligned_vaddr + (size - 1);
	bool is_user_space = mm_is_user_space(aligned_vaddr);

	if (is_user_space !=
		mm_is_user_space(vaddr_limit))
		return KM_RESULT_INVALID_ARGS;

	if (!(flags & MMAP_IGNORE_VMR)) {
		if (is_user_space) {
			ps_vmr_t *vmr;

			vmr = static_cast<ps_vmr_t *>(ctxt->vmr_tree.find(vaddr));

			if (!vmr)
				return KM_RESULT_INVALID_ARGS;

			if (size)
				return KM_RESULT_INVALID_ARGS;

			aligned_size = PGCEIL(vmr->size);

			ctxt->vmr_tree.remove(vmr);
			kfxx::destroy_at<ps_vmr_t>(vmr);
			kima_free(&ctxt->kima_vmr_pool, vmr);
		}
	}
	kh_unmmap(ctxt, aligned_vaddr, aligned_size, flags);
	return KM_RESULT_OK;
}
KI_EXPORT_IMAGE_SYMBOL(mm_unmmap);

PBOS_NODISCARD km_result_t mm_merge_mapped_area(
	mm_context_t *context,
	void *vaddr_a,
	void *vaddr_b) {
	if (vaddr_a >= vaddr_b)
		return KM_RESULT_INVALID_ARGS;

	ps_vmr_t *vmr_a, *vmr_b;

	vmr_a = static_cast<ps_vmr_t *>(context->vmr_tree.find(vaddr_a));
	if (!vmr_a)
		return KM_RESULT_INVALID_ARGS;

	vmr_b = static_cast<ps_vmr_t *>(context->vmr_tree.find(vaddr_b));
	if (!vmr_b)
		return KM_RESULT_INVALID_ARGS;

	if (((char *)vmr_a->rb_value) + vmr_a->size != vmr_b->rb_value)
		return KM_RESULT_INVALID_ARGS;

	vmr_a->size += vmr_b->size;

	context->vmr_tree.remove(vmr_b);
	kfxx::destroy_at<ps_vmr_t>(vmr_b);
	kima_free(&context->kima_vmr_pool, vmr_b);

	return KM_RESULT_OK;
}
KI_EXPORT_IMAGE_SYMBOL(mm_merge_mapped_area);

PBOS_NODISCARD km_result_t mm_split_mapped_area(
	mm_context_t *context,
	void *area_vaddr,
	void *split_point) {
	if (area_vaddr >= split_point)
		return KM_RESULT_INVALID_ARGS;

	ps_vmr_t *vmr;

	vmr = static_cast<ps_vmr_t *>(context->vmr_tree.find(area_vaddr));
	if (!vmr)
		return KM_RESULT_INVALID_ARGS;

	if (((char *)vmr->rb_value) + vmr->size <= split_point)
		return KM_RESULT_INVALID_ARGS;

	ps_vmr_t *new_vmr = (ps_vmr_t *)kima_alloc(&context->kima_vmr_pool, sizeof(ps_vmr_t), alignof(ps_vmr_t));
	if (!new_vmr)
		return KM_RESULT_NO_MEM;
	kfxx::construct_at<ps_vmr_t>(new_vmr);

	new_vmr->rb_value = split_point;
	new_vmr->size = (((char *)vmr->rb_value) + vmr->size) - (char *)split_point;
	new_vmr->access = vmr->access;

	context->vmr_tree.insert_unwrap(new_vmr);

	vmr->size = ((char *)split_point) - (char *)area_vaddr;

	return KM_RESULT_OK;
}
KI_EXPORT_IMAGE_SYMBOL(mm_split_mapped_area);

km_result_t mm_set_page_access(
	mm_context_t *context,
	void *vaddr,
	size_t size,
	mm_pgaccess_t access) {
	if (!context)
		return KM_RESULT_INVALID_ARGS;
	if (!size)
		return KM_RESULT_INVALID_ARGS;
	void *aligned_vaddr = (void *)PGFLOOR(vaddr);
	size_t aligned_size = PGCEIL(size);
	char *vaddr_limit = (char *)aligned_vaddr + (size - 1);
	bool is_user_space = mm_is_user_space(aligned_vaddr);

	if (is_user_space !=
		mm_is_user_space(vaddr_limit))
		return KM_RESULT_INVALID_ARGS;

	if (is_user_space) {
		ps_vmr_t *vmr;

		vmr = static_cast<ps_vmr_t *>(context->vmr_tree.find(vaddr));

		if (!vmr)
			return KM_RESULT_INVALID_ARGS;

		if (vmr->size != size)
			return KM_RESULT_INVALID_ARGS;
		vmr->access = access;
	}
	kh_set_page_access(context, vaddr, size, access);
	return KM_RESULT_OK;
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
