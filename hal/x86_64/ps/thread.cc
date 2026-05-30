#include <pbos/kd/logger.h>
#include <hal/x86_64/proc.hh>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/allocator.hh>
#include <pbos/kfxx/scope_guard.hh>

void ps_user_thread_init(ps_tcb_t *tcb) {
	tcb->context->rflags |= (1 << 9);  // IF
	tcb->context->cs = SELECTOR_UCODE;
	tcb->context->ds = SELECTOR_UDATA;
	tcb->context->ss = SELECTOR_UDATA;
	tcb->context->es = SELECTOR_UDATA;
	tcb->context->gs = SELECTOR_UDATA;
}

void ps_thread_set_entry(ps_tcb_t *tcb, void *ptr) {
	tcb->context->rip = ptr;
}

void ki_thread_set_stack(ps_tcb_t *tcb, void *ptr, size_t size) {
	kd_assert(tcb->context);
	const void *sp = ((char *)ptr) + size;
	tcb->stack = ptr;
	tcb->stack_size = size;
	tcb->context->rsp = (uint64_t)sp;
	tcb->context->rbp = (uint64_t)sp;
}

void ki_thread_set_kernel_stack(ps_tcb_t *tcb, void *ptr, size_t size) {
	kd_assert(tcb->context);
	const void *sp = ((char *)ptr) + size;
	tcb->kernel_stack = ptr;
	tcb->kernel_stack_size = size;
	tcb->context->rsp0 = (uint64_t)sp;
}

km_result_t ps_thread_alloc_stack(ps_tcb_t *tcb, size_t size) {
	const size_t page_size = mm_get_page_size();

	km_result_t result;
	ps_pcb_t *pcb = tcb->parent;

	char *ptr;
	if (!(ptr = (char *)mm_vmalloc(
			  pcb->mm_context,
			  (void *)USER_VBASE,
			  (void *)USER_VTOP,
			  size,
			  MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE | MM_PAGE_USER,
			  0))) {
		return KM_RESULT_NO_MEM;
	}

	{
		size_t i = 0;

		kfxx::scope_guard release_pages_guard([pcb, size, &i, ptr, page_size]() noexcept {
			if (i) {
				klog_printf("Freeing stack page: %p-%p", ptr, ptr + (i - page_size));
				mm_vmfree(pcb->mm_context, ptr, (i - page_size));
			}
		});

		for (; i < size; i += page_size) {
			void *pg = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);

			if (!pg) {
				klog_printf("Error allocating physical page: %p", ptr + i);
				--i;
				return KM_RESULT_NO_MEM;
			}

			kfxx::deferred release_pg_guard([pg]() noexcept {
				mm_unpin_page(pg);
			});

			// User mmap does not increase the kernel reference count.
			if (KM_FAILED(result = mm_mmap(pcb->mm_context, ptr + i, pg, page_size, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE | MM_PAGE_USER, 0))) {
				return result;
			}

			if (i)
				km_unwrap_result(mm_merge_mapped_area(pcb->mm_context, ptr, ptr + i));
		}

		release_pages_guard.release();
	}

	ki_thread_set_stack(tcb, ptr, size);

	return KM_RESULT_OK;
}

km_result_t ps_thread_alloc_kernel_stack(ps_tcb_t *tcb, size_t size) {
	const size_t page_size = mm_get_page_size();

	km_result_t result;
	ps_pcb_t *pcb = tcb->parent;

	char *ptr;
	if (!(ptr = (char *)mm_kvmalloc(
			  pcb->mm_context,
			  size,
			  MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE,
			  VMALLOC_ATOMIC))) {
		return KM_RESULT_NO_MEM;
	}

	{
		size_t i = 0;

		kfxx::scope_guard release_pages_guard([pcb, size, &i, ptr, page_size]() noexcept {
			for (size_t j = 0; j < i; j += page_size) {
				klog_printf("Freeing kernel stack page: %p", ptr + j);
				mm_vmfree(pcb->mm_context, ptr + j, page_size);
			}
		});

		for (; i < size; i += page_size) {
			void *pg = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);

			if (!pg) {
				return KM_RESULT_NO_MEM;
			}

			if (KM_FAILED(result = mm_mmap(pcb->mm_context, ptr + i, pg, page_size, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, MM_MMAP_NO_INC_RC))) {
				return result;
			}
		}

		release_pages_guard.release();
	}

	ki_thread_set_kernel_stack(tcb, ptr, size);

	// stub
	if (!ps_global_proc_set.size()) {
		hali_tss_storage_ptr[ps_get_cur_cpuid()].rsp0 = tcb->context->rsp0;
	}

	return KM_RESULT_OK;
}
