#include <hal/i386/proc.hh>

void kn_thread_destructor(om_object_t *obj) {
	hn_thread_cleanup(PBOS_CONTAINER_OF(ps_tcb_t, object_header, obj));
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
	om_init_object(&(t->object_header), ps_thread_class, 0);
	t->parent = pcb;
	t->context->eflags |= (1 << 9);  // IF

	return t;
}

thread_id_t ps_create_thread(
	ps_proc_access_t access,
	ps_pcb_t *pcb,
	size_t stacksize) {
	if (!stacksize)
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
	while (thread->stacksize) {
		mm_pgfree(mm_getmap(thread->parent->mm_context, thread->stack, NULL));
		((char *&)thread->stack) += PAGESIZE;
		thread->stacksize -= PAGESIZE;
	}
}

void ps_thread_set_entry(ps_tcb_t *tcb, void *ptr) {
	tcb->context->eip = ptr;
}

void kn_thread_setstack(ps_tcb_t *tcb, void *ptr, size_t size) {
	kd_assert(tcb->context);
	const void *sp = ((char *)ptr) + size;
	tcb->context->esp = (uint32_t)sp;
	tcb->context->ebp = (uint32_t)sp;
}

km_result_t ps_thread_allocstack(ps_tcb_t *tcb, size_t size) {
	km_result_t result;
	ps_pcb_t *pcb = tcb->parent;

	tcb->stacksize = size;
	if (!(tcb->stack = mm_vmalloc(
			  pcb->mm_context,
			  (void *)UFREE_VBASE,
			  (void *)UFREE_VTOP,
			  tcb->stacksize,
			  PAGE_MAPPED | PAGE_READ | PAGE_WRITE | PAGE_USER,
			  0))) {
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	}

	for (size_t i = 0; i < tcb->stacksize; i += PAGESIZE) {
		void *pg = mm_pgalloc(MM_PMEM_AVAILABLE);

		if (!pg) {
			do {
				mm_pgfree(mm_getmap(pcb->mm_context, ((char *)tcb->stack) + i, NULL));
			} while (--i);
			mm_vmfree(pcb->mm_context, tcb->stack, size);
			return KM_MAKEERROR(KM_RESULT_NO_MEM);
		}

		if (KM_FAILED(result = mm_mmap(pcb->mm_context, ((char *)tcb->stack) + i, pg, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE | PAGE_USER, 0))) {
			do {
				mm_pgfree(mm_getmap(pcb->mm_context, ((char *)tcb->stack) + i, NULL));
			} while (--i);
			mm_vmfree(pcb->mm_context, tcb->stack, size);
			return result;
		}
	}

	kn_thread_setstack(tcb, tcb->stack, tcb->stacksize);

	return KM_RESULT_OK;
}
