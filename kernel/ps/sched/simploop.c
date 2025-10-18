#include <pbos/km/proc.h>

PBOS_EXTERN_C_BEGIN

km_result_t ps_simploop_sched_init(ps_sched_t *sched) {
	return KM_RESULT_OK;
}

void ps_simploop_sched_deinit(ps_sched_t *sched) {
}

km_result_t ps_simploop_sched_prepare_proc(ps_sched_t *sched, ps_pcb_t *proc) {
	return KM_RESULT_OK;
}

km_result_t ps_simploop_sched_prepare_thread(ps_sched_t *sched, ps_tcb_t *thread) {
	return KM_RESULT_OK;
}

void ps_simploop_sched_drop_proc(ps_sched_t *sched, ps_pcb_t *proc) {
}

void ps_simploop_sched_drop_thread(ps_sched_t *sched, ps_tcb_t *thread) {
}

ps_tcb_t *ps_simploop_sched_next_thread(ps_sched_t *sched, ps_euid_t cur_euid, ps_pcb_t *cur_proc, ps_tcb_t *cur_thread) {
	ps_tcb_t *next_thread;

	if (!cur_thread) {
		ps_pcb_t *next_proc;
		next_proc = ps_global_proc_set_begin();
		next_thread = ps_proc_thread_set_begin(next_proc);
		ps_cur_proc_per_eu[cur_euid] = next_proc;
	} else {
		next_thread = ps_proc_thread_set_next(cur_proc, cur_thread);

		if (!next_thread) {
			ps_pcb_t *next_proc = ps_global_proc_set_next(cur_proc);
			if (!next_proc) {
				next_proc = ps_global_proc_set_begin();
			}

			ps_cur_proc_per_eu[cur_euid] = next_proc;
			next_thread = ps_proc_thread_set_begin(next_proc);
		}
	}

	return next_thread;
}

ps_sched_t ps_simploop_sched = {
	.init = ps_simploop_sched_init,
	.deinit = ps_simploop_sched_deinit,
	.prepare_proc = ps_simploop_sched_prepare_proc,
	.prepare_thread = ps_simploop_sched_prepare_thread,
	.drop_proc = ps_simploop_sched_drop_proc,
	.drop_thread = ps_simploop_sched_drop_thread,
	.next_thread = ps_simploop_sched_next_thread
};

PBOS_EXTERN_C_END
