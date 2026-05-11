#include <pbos/km/logger.h>
#include <hal/x86_64/proc.hh>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/allocator.hh>
#include <pbos/kfxx/scope_guard.hh>

void ki_destroy_thread(ps_tcb_t *tcb) {
	if (tcb->parent) {
		ps_cur_sched->drop_thread(ps_cur_sched, tcb);
		tcb->parent->thread_set.remove(tcb);
	}
	while (tcb->stack_size) {
		mm_pgfree(mm_getmap(tcb->parent->mm_context, tcb->stack, NULL));
		((char *&)tcb->stack) += PAGESIZE;
		tcb->stack_size -= PAGESIZE;
	}

	if (tcb->context)
		ps_destroy_context(tcb->context);

	kfxx::destroy_and_release<ps_tcb_t>(kfxx::kernel_allocator(), tcb);
	mm_kfree(tcb);
}

ps_tcb_t *ps_alloc_tcb(ps_pcb_t *pcb) {
	ps_tcb_t *t = (ps_tcb_t *)kfxx::alloc_and_construct<ps_tcb_t>(kfxx::kernel_allocator());
	if (!t)
		return NULL;
	kfxx::scope_guard release_tcb_guard([t]() noexcept {
		ki_destroy_thread(t);
	});
	if (!(t->context = ps_alloc_context())) {
		mm_kfree(t);
		return NULL;
	}
	t->parent = pcb;
	t->context->rflags |= (1 << 9);	 // IF

	release_tcb_guard.release();

	return t;
}

ps_thread_id_t ps_create_thread(
	ps_proc_access_t access,
	ps_pcb_t *pcb,
	size_t stack_size) {
	if (!stack_size)
		return -1;

	ps_tcb_t *t = ps_alloc_tcb(pcb);
	// ps_pcb_t *p = ps_getpcb(pid);

	if ((!t) || (!pcb))
		return -1;

	km_result_t result = ps_cur_sched->prepare_thread(ps_cur_sched, t);
	if (KM_FAILED(result)) {
		// TODO: Do something to destroy the TCB.
		return -1;
	}

	pcb->thread_set.insert_unwrap(t);
}

void ps_user_thread_init(ps_tcb_t *tcb) {
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
	km_result_t result;
	ps_pcb_t *pcb = tcb->parent;

	char *ptr;
	if (!(ptr = (char *)mm_vmalloc(
			  pcb->mm_context,
			  (void *)USER_VBASE,
			  (void *)USER_VTOP,
			  size,
			  MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE | MM_PAGE_USER,
			  VMALLOC_ATOMIC))) {
		return KM_RESULT_NO_MEM;
	}

	{
		size_t i = 0;

		kfxx::scope_guard release_pages_guard([pcb, size, &i, ptr]() noexcept {
			klog_printf("Freeing stack page: %p-%p", ptr, ptr + (i - PAGESIZE));
			mm_vmfree(pcb->mm_context, ptr, (i - PAGESIZE));
		});

		for (; i < size; i += PAGESIZE) {
			void *pg = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);

			if (!pg) {
				klog_printf("Error allocating physical page: %p", ptr + i);
				--i;
				return KM_RESULT_NO_MEM;
			}

			if (KM_FAILED(result = mm_mmap(pcb->mm_context, ptr + i, pg, PAGESIZE, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE | MM_PAGE_USER, MMAP_ATOMIC))) {
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

		kfxx::scope_guard release_pages_guard([pcb, size, &i, ptr]() noexcept {
			for (size_t j = 0; j < i; j += PAGESIZE) {
				klog_printf("Freeing kernel stack page: %p", ptr + j);
				mm_vmfree(pcb->mm_context, ptr + j, PAGESIZE);
			}
		});

		for (; i < size; i += PAGESIZE) {
			void *pg = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);

			if (!pg) {
				return KM_RESULT_NO_MEM;
			}

			if (KM_FAILED(result = mm_mmap(pcb->mm_context, ptr + i, pg, PAGESIZE, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE | MM_PAGE_USER, MMAP_ATOMIC))) {
				return result;
			}
		}

		release_pages_guard.release();
	}

	ki_thread_set_kernel_stack(tcb, ptr, size);

	// stub
	if (!ps_global_proc_set.size()) {
		hn_tss_storage_ptr[ps_get_cur_cpuid()].rsp0 = tcb->context->rsp0;
	}

	return KM_RESULT_OK;
}
