#include <hal/i386/proc.h>

void kn_thread_destructor(om_object_t *obj) {
	hn_thread_cleanup(PB_CONTAINER_OF(ps_tcb_t, object_header, obj));
}

ps_tcb_t *kn_alloc_tcb(ps_pcb_t *pcb) {
	ps_tcb_t *t = mm_kmalloc(sizeof(ps_tcb_t));
	if (!t)
		return NULL;
	memset(t, 0, sizeof(ps_tcb_t));
	om_init_object(&(t->object_header), ps_thread_class);
	t->parent = pcb;

	return t;
}

thread_id_t ps_create_thread(
	ps_proc_access_t access,
	ps_pcb_t *pcb,
	size_t stacksize) {
	if (!stacksize)
		return -1;

	ps_tcb_t *t = kn_alloc_tcb(pcb);
	// ps_pcb_t *p = ps_getpcb(pid);

	if ((!t) || (!pcb))
		return -1;

	kf_list_init((kf_list_node_t *)t, NULL, NULL);
	if (pcb->threads)
		kf_list_append((kf_list_node_t *)t, &(pcb->threads->list_header));
	pcb->threads = t;
}

void hn_thread_cleanup(ps_tcb_t *thread) {
	if (thread == thread->parent->threads)
		thread->parent->threads = PB_CONTAINER_OF(ps_tcb_t, list_header, kf_list_next((kf_list_node_t *)thread));
	kf_list_remove((kf_list_node_t *)thread);

	while (thread->stacksize) {
		mm_pgfree(mm_getmap(&thread->parent->mmctxt, UNPGADDR(thread->stack++)));
		thread->stacksize--;
	}
}

void kn_thread_setentry(ps_tcb_t *pcb, void *ptr) {
	pcb->context.eip = ptr;
}

void kn_thread_setstack(ps_tcb_t *pcb, void *ptr, size_t size) {
	const void *sp = ((char *)ptr) + size;
	pcb->context.esp = (uint32_t)sp;
	pcb->context.ebp = (uint32_t)sp;
	pcb->stack = PGROUNDDOWN(ptr);
}

km_result_t kn_thread_allocstack(ps_tcb_t *tcb, size_t size) {
	km_result_t result;
	ps_pcb_t *pcb = tcb->parent;

	tcb->stacksize = PGROUNDUP(size);
	tcb->stack = PGROUNDDOWN(mm_vmalloc(
		&pcb->mmctxt,
		(void *)UFREE_VBASE,
		(void *)UFREE_VTOP,
		UNPGSIZE(tcb->stacksize),
		PAGE_READ | PAGE_WRITE | PAGE_USER,
		0));

	for (pgsize_t i = 0; i < tcb->stacksize; ++i) {
		void *pg = mm_pgalloc(MM_PMEM_AVAILABLE);

		if (!pg) {
			do {
				mm_pgfree(mm_getmap(&pcb->mmctxt, UNPGADDR(tcb->stack + i)));
			} while (--i);
			mm_vmfree(&pcb->mmctxt, UNPGADDR(tcb->stack), size);
			return KM_RESULT_NO_MEM;
		}

		if (KM_FAILED(result = mm_mmap(&pcb->mmctxt, UNPGADDR(tcb->stack + i), (void *)PGFLOOR(pg), PAGESIZE, PAGE_READ | PAGE_WRITE | PAGE_USER, 0))) {
			do {
				mm_pgfree(mm_getmap(&pcb->mmctxt, UNPGADDR(tcb->stack + i)));
			} while (--i);
			mm_vmfree(&pcb->mmctxt, UNPGADDR(tcb->stack), size);
			return result;
		}
	}

	tcb->context.ebp = (tcb->context.esp = (uint32_t)UNPGADDR(tcb->stack) + size - 4);

	return KM_RESULT_OK;
}
