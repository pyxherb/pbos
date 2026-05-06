#include <pbos/kn/km/proc.hh>
#include <pbos/kn/km/exec.hh>

PBOS_EXTERN_C_BEGIN

void ps_init() {
	hal_prepare_ps();

	ki_init_binldrs();

	km_result_t result = ps_set_sched(&ps_simploop_sched);
	if(KM_FAILED(result))
		km_panic("Error initializing initial scheduler");
}

void ki_init_binldrs() {
	for (km_init_binldr_registry_t *i = ki_builtin_binldrs; i->binldr; ++i) {
		km_register_binldr(&i->uuid, i->binldr);
	}
}

PBOS_EXTERN_C_END
