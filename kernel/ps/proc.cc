#include <string.h>
#include <pbos/ki/km/proc.hh>

PBOS_EXTERN_C_BEGIN

kfxx::rbtree_t<ps_proc_id_t> ps_global_proc_set;
ps_sched_t *ps_cur_sched = NULL;

ps_pcb_t *ps_get_cur_proc() {
	return ps_cur_proc_per_cpu[ps_get_cur_cpuid()];
}

ps_tcb_t *ps_get_cur_thread() {
	return ps_cur_thread_per_cpu[ps_get_cur_cpuid()];
}

mm_context_t *ps_mm_context_of(ps_pcb_t *pcb) {
	return pcb->mm_context;
}

km_result_t ps_set_sched(ps_sched_t *sched) {
	km_result_t result;

	result = sched->init(sched);

	if (KM_FAILED(result))
		return result;

	if (ps_cur_sched) {
		ps_cur_sched->deinit(ps_cur_sched);
	}

	ps_cur_sched = sched;

	return KM_RESULT_OK;
}

ps_pcb_t *ps_global_proc_set_begin() {
	return static_cast<ps_pcb_t *>(ps_global_proc_set.begin().node);
}

ps_pcb_t *ps_global_proc_set_next(ps_pcb_t *cur) {
	return static_cast<ps_pcb_t *>(ps_global_proc_set.get_next(cur, nullptr));
}

ps_tcb_t *ps_proc_thread_set_begin(ps_pcb_t *pcb) {
	return static_cast<ps_tcb_t *>(pcb->thread_set.begin().node);
}

ps_tcb_t *ps_proc_thread_set_next(ps_pcb_t *pcb, ps_tcb_t *cur) {
	return static_cast<ps_tcb_t *>(pcb->thread_set.get_next(cur, nullptr));
}

fs_fnode_t *ps_get_cwd(ps_pcb_t *pcb) {
	return pcb->cur_dir.get();
}

void ps_set_cwd(ps_pcb_t *pcb, fs_fnode_t *cwd_node) {
	pcb->cur_dir = cwd_node;
}

void ps_unset_cwd(ps_pcb_t *pcb) {
	pcb->cur_dir.reset();
}

PBOS_EXTERN_C_END
