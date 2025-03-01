#include <pbos/kn/km/proc.h>
#include <pbos/kn/km/exec.h>

static bool _ps_pcb_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	return PB_CONTAINER_OF(ps_pcb_t, node_header, x)->proc_id < PB_CONTAINER_OF(ps_pcb_t, node_header, y)->proc_id;
}

static void _ps_pcb_nodefree(kf_rbtree_node_t *x) {
	// stub
}

void ps_init() {
	hal_prepare_ps();

	if (!(ps_proc_class = om_register_class(&PROC_CLASSID, kn_proc_destructor)))
		km_panic("Error registering process kernel class");

	if (!(ps_thread_class = om_register_class(&THREAD_CLASSID, kn_thread_destructor)))
		km_panic("Error registering thread kernel class");

	kf_rbtree_init(
		&ps_global_proc_set,
		_ps_pcb_nodecmp,
		_ps_pcb_nodefree);

	kn_init_binldrs();

	km_result_t result = ps_set_sched(&ps_simploop_sched);
	if(KM_FAILED(result))
		km_panic("Error initializing initial scheduler");
}

void kn_init_binldrs() {
	kf_rbtree_init(
		&kn_registered_binldrs,
		kn_binldr_reg_nodecmp,
		kn_binldr_reg_nodefree);

	for (km_binldr_t **i = kn_builtin_binldrs; *i; ++i) {
		km_register_binldr(*i);
	}
}
