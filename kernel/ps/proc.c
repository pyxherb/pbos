#include <pbos/km/proc.h>

om_class_t *ps_proc_class = NULL, *ps_thread_class = NULL;
kf_rbtree_t ps_global_proc_set;
uint32_t ps_eu_num;
ps_sched_t *ps_cur_sched = NULL;

km_result_t ps_set_sched(ps_sched_t *sched) {
	km_result_t result;

	result = sched->init(sched);

	if(KM_FAILED(result))
		return result;

	if(ps_cur_sched) {
		ps_cur_sched->deinit(ps_cur_sched);
	}

	ps_cur_sched = sched;

	return KM_RESULT_OK;
}
