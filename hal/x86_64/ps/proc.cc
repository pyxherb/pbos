#include <pbos/fs/file.h>
#include <pbos/fs/fs.h>
#include <pbos/km/logger.h>
#include <hal/x86_64/proc.hh>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/allocator.hh>
#include <pbos/kfxx/scope_guard.hh>

PBOS_EXTERN_C_BEGIN

ps_pcb_t **ps_cur_proc_per_cpu;
ps_tcb_t **ps_cur_thread_per_cpu;

ps_ufd_t ps_alloc_fd(ps_pcb_t *pcb) {
	return pcb->last_fd++;
}

ps_ufcb_t *ps_alloc_ufcb(ps_pcb_t *pcb, fs_fcb_t *kernel_fcb, ps_ufd_t fd) {
	ps_ufcb_t *p = (ps_ufcb_t *)mm_kalloc(sizeof(ps_ufcb_t), alignof(ps_ufcb_t));
	kfxx::construct_at<ps_ufcb_t>(p);
	if (!p)
		return NULL;
	p->rb_value = fd;
	p->kernel_fcb = kernel_fcb;
	ps_add_ufcb(pcb, p);
	return p;
}

void ps_add_ufcb(ps_pcb_t *pcb, ps_ufcb_t *ufcb) {
	if (!pcb->ufcb_set.insert(ufcb))
		km_panic("Error inserting new UFCB");
}

void ps_remove_ufcb(ps_pcb_t *pcb, ps_ufcb_t *ufcb) {
	pcb->ufcb_set.remove(ufcb);
}

ps_ufcb_t *ps_lookup_ufcb(ps_pcb_t *pcb, ps_ufd_t fd) {
	return static_cast<ps_ufcb_t *>(pcb->ufcb_set.find(fd));
}

void ki_destroy_proc(ps_pcb_t *pcb) {
	if (pcb->flags & PS_PROC_P)
		ps_cur_sched->drop_proc(ps_cur_sched, pcb);
	if (pcb->mm_context)
		mm_free_context(pcb->mm_context);

	pcb->thread_set.clear([](decltype(pcb->thread_set)::node_t *node) noexcept {
		ki_destroy_thread(static_cast<ps_tcb_t *>(node));
	});

	kfxx::destroy_and_release<ps_pcb_t>(kfxx::kernel_allocator(), pcb);
}

void ps_create_proc(
	ps_pcb_t *pcb,
	ps_proc_id_t parent) {
	io::irq_disable_lock irq_disable_lock;
	if (ps_global_proc_set.find(pcb->rb_value))
		km_panic("Trying to create a new process with PCB with PID that is already used by a process");

	if (!ps_global_proc_set.insert(pcb))
		km_panic("Error inserting new PCB to global process set");
}

ps_pcb_t *ps_alloc_pcb() {
	ps_pcb_t *proc = (ps_pcb_t *)kfxx::alloc_and_construct<ps_pcb_t>(kfxx::kernel_allocator());

	if (!proc)
		return NULL;

	kfxx::scope_guard release_proc_guard([proc]() noexcept {
		ki_destroy_proc(proc);
	});

	if (!(proc->mm_context = (mm_context_t *)mm_kalloc(sizeof(mm_context_t), alignof(mm_context_t)))) {
		mm_kfree(proc);
		return NULL;
	}

	if (KM_FAILED(kh_mm_alloc_context(mm_get_cur_context(), &proc->mm_context))) {
		mm_kfree(proc->mm_context);
		mm_kfree(proc);
		return NULL;
	}

	if (KM_FAILED(ps_cur_sched->prepare_proc(ps_cur_sched, proc))) {
		mm_kfree(proc->mm_context);
		mm_kfree(proc);
		return NULL;
	}

	proc->last_thread_id = 0;
	proc->last_fd = 0;

	proc->flags = PS_PROC_P;

	release_proc_guard.release();

	return proc;
}

ps_pcb_t *ps_getpcb(ps_proc_id_t pid) {
	return static_cast<ps_pcb_t *>(ps_global_proc_set.find(pid));
}

void ps_add_thread(ps_pcb_t *proc, ps_tcb_t *thread) {
	io::irq_disable_lock irq_disable_lock;
	// stub: do some checks with the new thread id, such as checking if a thread with the id exists.
	thread->rb_value = ++proc->last_thread_id;
	if (!proc->thread_set.insert(thread))
		km_panic("Error adding new TCB to process #%u", proc->rb_value);
}

void ki_switch_to_user_process(ps_pcb_t *pcb) {
	mm_switch_context(pcb->mm_context);
}

void ki_switch_to_user_thread(ps_tcb_t *tcb) {
	ps_load_user_context(tcb->context);
}

void ki_switch_to_kernel_thread(ps_tcb_t *tcb) {
	ps_load_kernel_context(tcb->context);
}

ps_cpu_id_t ps_get_cur_cpuid() {
	return arch_storefs();
}

void ki_set_cur_cpuid(ps_cpu_id_t cpuid) {
	arch_loadfs(cpuid);
}

PBOS_EXTERN_C_END
