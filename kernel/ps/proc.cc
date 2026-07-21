#include <pbos/kh/ps/proc.h>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/km/symbol.hh>
#include <pbos/ki/mm/context.hh>
#include <pbos/ki/ps/proc.hh>
#include <pbos/ps/semaphore.hh>

PBOS_EXTERN_C_BEGIN

kfxx::rbtree_t<ps_proc_id_t> ps_global_proc_set;
ps::semaphore_t ps_global_proc_set_mutex;
ps_sched_t *ps_cur_sched = NULL;
ps_proc_id_t ki_min_free_pid = 0, ki_cur_max_pid = 0;

PBOS_API ps_ufd_t ps_alloc_fd(ps_pcb_t *pcb) {
	return pcb->last_fd++;
}

PBOS_API ps_ufcb_t *ps_alloc_ufcb(ps_pcb_t *pcb, fs_fcb_t *kernel_fcb, ps_ufd_t fd) {
	ps_ufcb_t *p = (ps_ufcb_t *)mm_kalloc(sizeof(ps_ufcb_t), alignof(ps_ufcb_t));
	kfxx::construct_at<ps_ufcb_t>(p);
	if (!p)
		return nullptr;
	p->rb_value = fd;
	p->kernel_fcb = kernel_fcb;
	ps_add_ufcb(pcb, p);
	return p;
}

PBOS_API void ps_add_ufcb(ps_pcb_t *pcb, ps_ufcb_t *ufcb) {
	kd_dbgcheck(pcb->ufcb_set.insert(ufcb), "Error inserting new UFCB with ID=%d", ufcb->rb_value);
}

PBOS_API void ps_remove_ufcb(ps_pcb_t *pcb, ps_ufcb_t *ufcb) {
	pcb->ufcb_set.remove(ufcb);
}

PBOS_API ps_ufcb_t *ps_lookup_ufcb(ps_pcb_t *pcb, ps_ufd_t fd) {
	return static_cast<ps_ufcb_t *>(pcb->ufcb_set.find(fd));
}

PBOS_API fs_fcb_t *ps_kfcb_of_ufcb(ps_ufcb_t *ufcb) {
	return ufcb->kernel_fcb;
}

void ki_destroy_pcb(ps_pcb_t *pcb) {
	{
		// Make sure only remove when the PCB is in the global process set.
		ps::write_semaphore_guard g(ps_global_proc_set_mutex);
		if (auto node = ps_global_proc_set.find(pcb->rb_value); node) {
			ps_global_proc_set.remove(node);

			if (pcb->rb_value < ki_min_free_pid) {
				ki_min_free_pid = pcb->rb_value;
			}
			// Update the current maximum PID.
			if (pcb->rb_value == ki_cur_max_pid) {
				auto now_max = ps_global_proc_set.find_max_lteq(ki_cur_max_pid);
				if (!now_max)
					ki_cur_max_pid = 0;
				else
					ki_cur_max_pid = now_max->rb_value;
			}
		}
	}
	if (pcb->flags & PS_PROC_P)
		ps_cur_sched->drop_proc(ps_cur_sched, pcb);
	if (pcb->mm_context)
		mm_free_context(pcb->mm_context);

	pcb->thread_set.clear([](decltype(pcb->thread_set)::node_t *node) noexcept {
		ki_destroy_tcb(static_cast<ps_tcb_t *>(node));
	});

	kfxx::destroy_and_release<ps_pcb_t>(kfxx::kernel_allocator(), pcb);
}

PBOS_API km_result_t ps_create_proc(
	ps_pcb_t *pcb,
	ps_proc_id_t parent) {
	ps::write_semaphore_guard g(ps_global_proc_set_mutex);

	pcb->rb_value = PS_PROC_ID_MAX;

	for (ps_proc_id_t i = ki_min_free_pid; i != PS_PROC_ID_MAX; ++i) {
		if (!ps_global_proc_set.find(i)) {
			if (i >= ki_cur_max_pid) {
				ki_cur_max_pid = i;
			}
			ki_min_free_pid = i + 1;
			pcb->rb_value = i;
			break;
		}
	}

	if (pcb->rb_value == PS_PROC_ID_MAX) {
		for (ps_proc_id_t i = 0; i != ki_min_free_pid; ++i) {
			if (!ps_global_proc_set.find(i)) {
				if (i >= ki_cur_max_pid) {
					ki_cur_max_pid = i;
				}
				ki_min_free_pid = i + 1;
				pcb->rb_value = i;
				break;
			}
		}
	}

	if (pcb->rb_value == PS_PROC_ID_MAX) {
		return KM_RESULT_NO_SLOT;
	}

	kd_dbgcheck(ps_global_proc_set.insert(pcb), "Error adding new PCB with PID=%u", pcb->rb_value);
	return KM_RESULT_OK;
}

PBOS_API ps_pcb_t *ps_alloc_pcb() {
	mm_context_t *mm_context = mm_get_cur_context();

	ps_pcb_t *proc = (ps_pcb_t *)kfxx::alloc_and_construct<ps_pcb_t>(kfxx::kernel_allocator());

	if (!proc)
		return nullptr;

	kfxx::scope_guard release_proc_guard([proc]() noexcept {
		ki_destroy_pcb(proc);
	});

	if (KM_FAILED(mm_alloc_context(mm_context, &proc->mm_context)))
		return nullptr;

	proc->mm_context->pcb = proc;

	if (KM_FAILED(ps_cur_sched->prepare_proc(ps_cur_sched, proc)))
		return nullptr;

	proc->last_thread_id = 0;
	proc->last_fd = 0;

	proc->flags = PS_PROC_P;

	release_proc_guard.release();

	return proc;
}

PBOS_API ps_pcb_t *ps_lookup_pcb(ps_proc_id_t pid) {
	return static_cast<ps_pcb_t *>(ps_global_proc_set.find(pid));
}

PBOS_API ps_proc_id_t ps_pid_of(ps_pcb_t *pcb) {
	return pcb->rb_value;
}

PBOS_API void ps_add_thread(ps_pcb_t *proc, ps_tcb_t *thread) {
	ps::write_semaphore_guard g(proc->thread_set_semaphore);
	// stub: do some checks with the new thread id, such as checking if a thread with the id exists.
	thread->rb_value = ++proc->last_thread_id;
	kd_dbgcheck(proc->thread_set.insert(thread), "Error adding thread with PID=%u, TID=%u", proc->rb_value, thread->rb_value);
}

void ki_switch_to_user_process(ps_pcb_t *pcb) {
	kh_switch_to_user_process(pcb);
}

void ki_switch_to_user_thread(ps_tcb_t *tcb) {
	kh_switch_to_user_thread(tcb);
}

void ki_switch_to_kernel_thread(ps_tcb_t *tcb) {
	kh_switch_to_kernel_thread(tcb);
}

PBOS_API ps_cpuid_t ps_get_cur_cpuid() {
	return kh_get_cur_cpuid();
}

void ki_set_cur_cpuid(ps_cpuid_t cpuid) {
	kh_set_cur_cpuid(cpuid);
}

PBOS_API ps_pcb_t *ps_get_cur_proc() {
	if (ps_cur_proc_per_cpu)
		return ps_cur_proc_per_cpu[ps_get_cur_cpuid()];
	return nullptr;
}

PBOS_API ps_tcb_t *ps_get_cur_thread() {
	if (ps_cur_thread_per_cpu)
		return ps_cur_thread_per_cpu[ps_get_cur_cpuid()];
	return nullptr;
}

PBOS_API mm_context_t *ps_mm_context_of(ps_pcb_t *pcb) {
	return pcb->mm_context;
}

PBOS_API fs_fnode_t *ps_get_cwd(ps_pcb_t *pcb) {
	return pcb->cur_dir.get();
}

PBOS_API void ps_set_cwd(ps_pcb_t *pcb, fs_fnode_t *cwd_node) {
	pcb->cur_dir = cwd_node;
}

PBOS_API void ps_unset_cwd(ps_pcb_t *pcb) {
	pcb->cur_dir.reset();
}

PBOS_API void ps_yield_cur_thread() {
	kh_yield_cur_thread();
}

PBOS_EXTERN_C_END
