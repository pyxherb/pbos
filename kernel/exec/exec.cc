#include <hal/i386/proc.h>
#include <pbos/kn/km/exec.h>
#include <string.h>

PBOS_EXTERN_C_BEGIN

typedef struct _kn_binldr_reg_t {
	kf_rbtree_node_t tree_header;
	uuid_t uuid;
	km_binldr_t binldr;
} kn_binldr_reg_t;

kf_rbtree_t kn_registered_binldrs;

static proc_id_t _last_proc_id = 0;

proc_id_t kn_alloc_proc_id() {
	return _last_proc_id++;
}

km_result_t km_register_binldr(km_binldr_t *binldr) {
	kn_binldr_reg_t *reg = (kn_binldr_reg_t*)mm_kmalloc(sizeof(kn_binldr_reg_t));
	if (!reg)
		return KM_MAKEERROR(KM_RESULT_NO_MEM);

	// Initialize the registry
	memset(reg, 0, sizeof(kn_binldr_reg_t));
	memcpy(&(reg->binldr), binldr, sizeof(km_binldr_t));

	kf_rbtree_insert(&kn_registered_binldrs, &reg->tree_header);

	return KM_RESULT_OK;
}

km_result_t km_exec(
	proc_id_t parent,
	se_uid_t uid,
	fs_fcb_t *file_fp,
	proc_id_t *pid_out) {
	km_result_t result;

	proc_id_t new_proc_id = kn_alloc_proc_id();
	if(new_proc_id < 0)
		return KM_MAKEERROR(KM_RESULT_NO_SLOT);

	ps_pcb_t *pcb = kn_alloc_pcb();
	if (!pcb) {
		result = KM_MAKEERROR(KM_RESULT_NO_MEM);
		goto failed;
	}

	kf_rbtree_foreach(i, &kn_registered_binldrs) {
		kn_binldr_reg_t *binldr = PBOS_CONTAINER_OF(kn_binldr_reg_t, tree_header, i);

		if (KM_SUCCEEDED(result = binldr->binldr.load_exec(pcb, file_fp))) {
			pcb->proc_id = kn_alloc_proc_id();
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

bool kn_binldr_reg_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	const kn_binldr_reg_t *_x = (const kn_binldr_reg_t *)x,
						  *_y = (const kn_binldr_reg_t *)y;

	return uuid_lt(&_x->uuid, &_y->uuid);
}

void kn_binldr_reg_nodefree(kf_rbtree_node_t *p) {
	// TODO: Do some freeing operations?
}

PBOS_EXTERN_C_END
