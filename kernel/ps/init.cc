#include <pbos/kn/km/proc.hh>
#include <pbos/kn/km/exec.hh>

PBOS_EXTERN_C_BEGIN

void ps_init() {
	hal_prepare_ps();

	uuid_t uuid;
	uuid = PROC_CLASSID;

	if (!(ps_proc_class = om_register_class(&uuid, kn_proc_destructor)))
		km_panic("Error registering process kernel class");

	uuid = THREAD_CLASSID;

	if (!(ps_thread_class = om_register_class(&uuid, kn_thread_destructor)))
		km_panic("Error registering thread kernel class");

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

PBOS_EXTERN_C_END
