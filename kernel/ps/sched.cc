#include <pbos/ki/km/proc.hh>
#include <pbos/ki/km/symbol.hh>

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
KI_EXPORT_IMAGE_SYMBOL(ps_set_sched);

ps_pcb_t *ps_global_proc_set_begin() {
	return static_cast<ps_pcb_t *>(ps_global_proc_set.begin().node);
}
KI_EXPORT_IMAGE_SYMBOL(ps_global_proc_set_begin);

ps_pcb_t *ps_global_proc_set_next(ps_pcb_t *cur) {
	return static_cast<ps_pcb_t *>(ps_global_proc_set.get_next(cur, nullptr));
}
KI_EXPORT_IMAGE_SYMBOL(ps_global_proc_set_next);

ps_tcb_t *ps_proc_thread_set_begin(ps_pcb_t *pcb) {
	return static_cast<ps_tcb_t *>(pcb->thread_set.begin().node);
}
KI_EXPORT_IMAGE_SYMBOL(ps_proc_thread_set_begin);

ps_tcb_t *ps_proc_thread_set_next(ps_pcb_t *pcb, ps_tcb_t *cur) {
	return static_cast<ps_tcb_t *>(pcb->thread_set.get_next(cur, nullptr));
}
KI_EXPORT_IMAGE_SYMBOL(ps_proc_thread_set_next);
