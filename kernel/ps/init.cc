#include <pbos/ki/km/proc.hh>
#include <pbos/ki/ps/exec.hh>

PBOS_EXTERN_C_BEGIN

void ki_ps_init() {
	hal_prepare_ps();

	ki_init_binldrs();

	km_result_t result = ps_set_sched(&ps_simploop_sched);
	if(KM_FAILED(result))
		km_panic("Error initializing initial scheduler");
}

void ki_init_binldrs() {
	for (km_init_binldr_registry_t *i = ki_builtin_binldrs; i->ops; ++i) {
		ps_register_binldr(&i->uuid, i->ops);
	}
}

PBOS_EXTERN_C_END
