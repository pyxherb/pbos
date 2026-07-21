#include <pbos/kd/logger.h>
#include <pbos/kfxx/allocator.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/ps/proc.hh>

PBOS_EXTERN_C_BEGIN

void ki_destroy_tcb(ps_tcb_t *tcb) {
	const size_t page_size = mm_get_page_size();

	if (tcb->parent) {
		ps::write_semaphore_guard g(tcb->parent->thread_set_semaphore);
		ps_cur_sched->drop_thread(ps_cur_sched, tcb);
		tcb->parent->thread_set.remove(tcb);
	}
	km_unwrap_result(mm_munmap(tcb->parent->mm_context, tcb->stack, tcb->stack_size, 0));

	if (tcb->context)
		ps_destroy_context(tcb->context);

	// TODO: Release the kernel stack.

	kfxx::destroy_and_release<ps_tcb_t>(kfxx::kernel_allocator(), tcb);
	mm_kfree(tcb);
}

ps_tcb_t *ps_alloc_tcb(ps_pcb_t *pcb) {
	ps_tcb_t *t = (ps_tcb_t *)kfxx::alloc_and_construct<ps_tcb_t>(kfxx::kernel_allocator());
	if (!t)
		return nullptr;
	kfxx::scope_guard release_tcb_guard([t]() noexcept {
		ki_destroy_tcb(t);
	});
	if (!(t->context = ps_alloc_context()))
		return nullptr;
	t->parent = pcb;

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

	return KM_RESULT_OK;
}

PBOS_EXTERN_C_END
