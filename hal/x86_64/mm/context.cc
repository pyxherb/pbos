#include <arch/x86_64/paging.h>
#include <string.h>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/allocator.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/mp/misc.hh>
#include "../mm.hh"

PBOS_EXTERN_C_BEGIN

mm_context_t hali_kernel_mm_context;
mm_context_t *mm_kernel_context = &hali_kernel_mm_context;

void kh_mm_copy_global_mappings(mm_context_t *dest, const mm_context_t *src) {
	if (dest == src)
		return;
	memcpy(
		((arch_pml4te_t *)dest->page_table) + PML4X(KSPACE_VBASE),
		((arch_pml4te_t *)src->page_table) + PML4X(KSPACE_VBASE),
		sizeof(arch_pml4te_t) * (PML4X(UINTPTR_MAX - KSPACE_VBASE + 1)));
}

void kh_mm_sync_global_mappings(const mm_context_t *src) {
	for (ps_cpuid_t i = 0; i < mp_num_total_cpu; ++i) {
		mm_context_t *cur_context = mm_cur_contexts[i];

		if (cur_context == src)
			continue;

		kh_mm_copy_global_mappings(cur_context, src);
	}
}

km_result_t kh_mm_alloc_context(mm_context_t *context, mm_context_t **new_context_out) {
	km_result_t result;

	mm_context_t *new_context;

	if (!(new_context = (mm_context_t *)mm_kalloc(sizeof(mm_context_t), alignof(mm_context_t))))
		return KM_RESULT_NO_MEM;

	kfxx::scope_guard free_new_context_guard([new_context]() noexcept {
		mm_kfree(new_context);
	});

	void *pml4t_paddr = NULL,
		 *pml4t_vaddr = NULL;

	if (!(pml4t_paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))) {
		return KM_RESULT_NO_MEM;
	}
	kfxx::scope_guard free_pml4t_paddr_guard([pml4t_paddr]() noexcept {
		mm_unpin_page(pml4t_paddr);
	});

	if (!(pml4t_vaddr = mm_kvmalloc(mm_get_cur_context(), PAGESIZE, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, 0))) {
		return KM_RESULT_NO_MEM;
	}
	kfxx::scope_guard free_pml4t_vaddr_guard([pml4t_vaddr]() noexcept {
		mm_vmfree(mm_get_cur_context(), pml4t_vaddr, PAGESIZE);
	});

	kfxx::construct_at<mm_context_t>(new_context);
	if (KM_FAILED(result = mm_mmap(mm_get_cur_context(), pml4t_vaddr, pml4t_paddr, PAGESIZE, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, MMAP_NO_INC_RC))) {
		return result;
	}

	free_pml4t_paddr_guard.release();
	free_pml4t_vaddr_guard.release();

	memset(pml4t_vaddr, 0, PAGESIZE);
	new_context->page_table = (arch_pml4te_t *)pml4t_vaddr;

	kh_mm_copy_global_mappings(new_context, mm_get_cur_context());

	free_new_context_guard.release();

	*new_context_out = new_context;

	// ki_mm_copy_global_mappings(context, mm_kernel_context);
	return KM_RESULT_OK;
}

void mm_free_context(mm_context_t *context) {
	// TODO: Free associated pages.
	km_unwrap_result(mm_unmmap(context, 0, USER_SIZE, MMAP_IGNORE_VMR));

	while (context->vmr_tree.size()) {
		auto vmr = static_cast<mm_vmr_t *>(context->vmr_tree.begin().node);
		context->vmr_tree.remove(vmr);
		kfxx::destroy_at<mm_vmr_t>(vmr);
		kima_free(&context->kima_vmr_pool, vmr);
	}

	// Free memory blocks allocated by KIMA.
	// We must free all pages before the top-level page table is unmapped.
	kima_free_pool(&context->kima_common_pool);
	kima_free_pool(&context->kima_vmr_pool);

	// Free the top-level page table.
	auto pml4t = mm_getmap(mm_get_cur_context(), context->page_table, nullptr);
	kd_assert(pml4t);
	mm_unpin_page(pml4t);

	kfxx::destroy_and_release<mm_context_t>(kfxx::kernel_allocator(), context);
}

void mm_switch_context(mm_context_t *context) {
	kd_assert(context);
	io::local_irq_lock irq_lock;

	mm_context_t *prev_context = mm_get_cur_context();
	mm_cur_contexts[ps_get_cur_cpuid()] = context;
	kh_mm_copy_global_mappings(context, prev_context);
	// ki_mm_copy_global_mappings(mm_kernel_context, prev_context);
	// asm volatile("xchg %bx, %bx");
	arch_lpgtab(PGROUNDDOWN(mm_getmap(prev_context, context->page_table, NULL)));
}

PBOS_EXTERN_C_END
