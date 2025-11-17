#include <pbos/km/logger.h>
#include <hal/i386/proc.hh>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/scope_guard.hh>

void kn_thread_destructor(om_object_t *obj) {
	ps_tcb_t *tcb = static_cast<ps_tcb_t *>(obj);
	hn_thread_cleanup(tcb);
	kfxx::destroy_at<ps_tcb_t>(tcb);
	mm_kfree(tcb);
}

ps_tcb_t *ps_alloc_tcb(ps_pcb_t *pcb) {
	ps_tcb_t *t = (ps_tcb_t *)mm_kmalloc(sizeof(ps_tcb_t), alignof(ps_tcb_t));
	if (!t)
		return NULL;
	memset(t, 0, sizeof(ps_tcb_t));
	if (!(t->context = (ps_user_context_t *)mm_kmalloc(sizeof(ps_user_context_t), alignof(ps_user_context_t)))) {
		mm_kfree(t);
		return NULL;
	}
	memset(t->context, 0, sizeof(ps_user_context_t));
	om_init_object(t, ps_thread_class, 0);
	t->parent = pcb;
	t->context->eflags |= (1 << 9);	 // IF

	return t;
}

thread_id_t ps_create_thread(
	ps_proc_access_t access,
	ps_pcb_t *pcb,
	size_t stack_size) {
	io::irq_disable_lock irq_disable_lock;
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

	pcb->thread_set.insert(t);
}

void hn_thread_cleanup(ps_tcb_t *thread) {
	ps_cur_sched->drop_thread(ps_cur_sched, thread);
	thread->parent->thread_set.remove(thread);
	while (thread->stack_size) {
		mm_pgfree(mm_getmap(thread->parent->mm_context, thread->stack, NULL));
		((char *&)thread->stack) += PAGESIZE;
		thread->stack_size -= PAGESIZE;
	}
}

void ps_user_thread_init(ps_tcb_t *tcb) {
	tcb->context->cs = SELECTOR_UCODE;
	tcb->context->ds = SELECTOR_UDATA;
	tcb->context->ss = SELECTOR_UDATA;
	tcb->context->es = SELECTOR_UDATA;
	tcb->context->gs = SELECTOR_UDATA;
}

void ps_thread_set_entry(ps_tcb_t *tcb, void *ptr) {
	tcb->context->eip = ptr;
}

void kn_thread_set_stack(ps_tcb_t *tcb, void *ptr, size_t size) {
	kd_assert(tcb->context);
	const void *sp = ((char *)ptr) + size;
	tcb->stack = ptr;
	tcb->stack_size = size;
	tcb->context->esp = (uint32_t)sp;
	tcb->context->ebp = (uint32_t)sp;
}

void kn_thread_set_kernel_stack(ps_tcb_t *tcb, void *ptr, size_t size) {
	kd_assert(tcb->context);
	const void *sp = ((char *)ptr) + size;
	tcb->kernel_stack = ptr;
	tcb->kernel_stack_size = size;
	tcb->context->esp0 = (uint32_t)sp;
}

km_result_t ps_thread_alloc_stack(ps_tcb_t *tcb, size_t size) {
	km_result_t result;
	ps_pcb_t *pcb = tcb->parent;

	char *ptr;
	if (!(ptr = (char *)mm_vmalloc(
			  pcb->mm_context,
			  (void *)UFREE_VBASE,
			  (void *)UFREE_VTOP,
			  size,
			  PAGE_MAPPED | PAGE_READ | PAGE_WRITE | PAGE_USER,
			  VMALLOC_ATOMIC))) {
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	}

	{
		size_t i = 0;

		kfxx::scope_guard release_pages_guard([pcb, size, &i, ptr]() noexcept {
			klog_printf("Freeing stack page: %p-%p", ptr, ptr + (i - PAGESIZE));
			mm_vmfree(pcb->mm_context, ptr, (i - PAGESIZE));
		});

		for (; i < size; i += PAGESIZE) {
			void *pg = mm_pgalloc(MM_PMEM_AVAILABLE);

			if (!pg) {
				klog_printf("Error allocating physical page: %p", ptr + i);
				--i;
				return KM_MAKEERROR(KM_RESULT_NO_MEM);
			}

			if (KM_FAILED(result = mm_mmap(pcb->mm_context, ptr + i, pg, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE | PAGE_USER, MMAP_ATOMIC))) {
				return result;
			}
		}

		release_pages_guard.release();
	}

	kn_thread_set_stack(tcb, ptr, size);

	return KM_RESULT_OK;
}

km_result_t ps_thread_alloc_kernel_stack(ps_tcb_t *tcb, size_t size) {
	km_result_t result;
	ps_pcb_t *pcb = tcb->parent;

	char *ptr;
	if (!(ptr = (char *)mm_vmalloc(
			  pcb->mm_context,
			  (void *)KSPACE_VBASE,
			  (void *)KSPACE_VTOP,
			  size,
			  PAGE_MAPPED | PAGE_READ | PAGE_WRITE,
			  VMALLOC_ATOMIC))) {
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
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
			void *pg = mm_pgalloc(MM_PMEM_AVAILABLE);

			if (!pg) {
				return KM_MAKEERROR(KM_RESULT_NO_MEM);
			}

			if (KM_FAILED(result = mm_mmap(pcb->mm_context, ptr + i, pg, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE | PAGE_USER, MMAP_ATOMIC))) {
				return result;
			}
		}

		release_pages_guard.release();
	}

	kn_thread_set_kernel_stack(tcb, ptr, size);

	// stub
	if (!ps_global_proc_set.size()) {
		hn_tss_storage_ptr[ps_get_cur_euid()].esp0 = tcb->context->esp0;
		hn_tss_storage_ptr[ps_get_cur_euid()].ss0 = SELECTOR_KDATA;
	}

	return KM_RESULT_OK;
}
