#include <pbos/kn/km/proc.h>

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
		next_proc = PBOS_CONTAINER_OF(ps_pcb_t, node_header, kf_rbtree_begin(&ps_global_proc_set));
		next_thread = PBOS_CONTAINER_OF(ps_tcb_t, node_header, kf_rbtree_begin(&next_proc->thread_set));
		ps_cur_proc_per_eu[cur_euid] = next_proc;

		next_thread = PBOS_CONTAINER_OF(ps_tcb_t, node_header, kf_rbtree_begin(&next_proc->thread_set));
	} else {
		kf_rbtree_node_t *next_thread_node = kf_rbtree_next(&cur_thread->node_header);

		if (!next_thread_node) {
			kf_rbtree_node_t *next_proc_node = kf_rbtree_next(&cur_proc->node_header);
			ps_pcb_t *next_proc;
			if (!next_proc_node) {
				next_proc = PBOS_CONTAINER_OF(ps_pcb_t, node_header, kf_rbtree_begin(&ps_global_proc_set));
			} else {
				next_proc = PBOS_CONTAINER_OF(ps_pcb_t, node_header, next_proc_node);
			}

			ps_cur_proc_per_eu[cur_euid] = next_proc;
			next_thread = PBOS_CONTAINER_OF(ps_tcb_t, node_header, kf_rbtree_begin(&next_proc->thread_set));
		} else {
			next_thread = PBOS_CONTAINER_OF(ps_tcb_t, node_header, next_thread_node);
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
