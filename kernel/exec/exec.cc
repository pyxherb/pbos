#include <string.h>
#include <hal/i386/proc.hh>
#include <pbos/hal/irq.hh>
#include <pbos/kn/km/exec.hh>

PBOS_EXTERN_C_BEGIN

kfxx::rbtree_t<kf_uuid_t> kn_registered_binldrs;

static proc_id_t _last_proc_id = 0;

proc_id_t kn_alloc_proc_id() {
	return _last_proc_id++;
}

km_result_t km_register_binldr(kf_uuid_t *uuid, km_binldr_t *binldr) {
	kn_binldr_registry_t *reg = (kn_binldr_registry_t *)mm_kmalloc(sizeof(kn_binldr_registry_t), alignof(kn_binldr_registry_t));
	if (!reg)
		return KM_MAKEERROR(KM_RESULT_NO_MEM);

	// Initialize the registry
	kfxx::construct_at<kn_binldr_registry_t>(reg);

	memcpy(&reg->rb_value, &uuid, sizeof(*uuid));
	memcpy(&reg->binldr, binldr, sizeof(km_binldr_t));

	kn_registered_binldrs.insert(reg);

	return KM_RESULT_OK;
}

km_result_t km_exec(
	proc_id_t parent,
	se_uid_t uid,
	fs_fcb_t *file_fp,
	proc_id_t *pid_out) {
	km_result_t result;

	proc_id_t new_proc_id = kn_alloc_proc_id();
	if (new_proc_id < 0)
		return KM_MAKEERROR(KM_RESULT_NO_SLOT);

	ps_pcb_t *pcb = ps_alloc_pcb();
	if (!pcb) {
		result = KM_MAKEERROR(KM_RESULT_NO_MEM);
		goto failed;
	}

	for(auto it = kn_registered_binldrs.begin(); it != kn_registered_binldrs.end(); ++it) {
		io::irq_disable_lock irq_disable_lock;

		if (KM_SUCCEEDED(result = static_cast<kn_binldr_registry_t *>(it.node)->binldr.load_exec(pcb, file_fp))) {
			io::irq_disable_lock irq_disable_lock;
			pcb->rb_value = kn_alloc_proc_id();
			ps_create_proc(pcb, parent);
			return KM_RESULT_OK;
		}

		if (result != KM_RESULT_UNSUPPORTED_EXECFMT) {
			goto failed;
		}
	}

	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_EXECFMT);
failed:
	if (pcb) {
		// TODO: Release PCB.
	}

	return result;
}

PBOS_EXTERN_C_END
