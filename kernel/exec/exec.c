#include <oicos/kn/km/exec.h>
#include <string.h>

typedef struct _kn_binldr_reg_t {
	kf_rbtree_node_t tree_header;
	uuid_t uuid;
	km_binldr_t binldr;
} kn_binldr_reg_t;

kf_rbtree_t kn_registered_binldrs;

int kn_binldr_reg_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
void kn_binldr_reg_nodefree(kf_rbtree_node_t *p);

km_result_t km_register_binldr(km_binldr_t *binldr) {
	kn_binldr_reg_t *reg = mm_kmalloc(sizeof(kn_binldr_reg_t));
	if (!reg)
		return KM_MAKEERROR(KM_RESULT_NO_MEM);

	// Initialize the registry
	memcpy(&(reg->binldr), binldr, sizeof(km_binldr_t));


	kf_rbtree_insert(&kn_registered_binldrs, &reg->tree_header);

	return KM_RESULT_OK;
}

km_result_t km_exec(
	proc_id_t parent,
	se_uid_t uid,
	om_handle_t file_handle,
	proc_id_t *pid_out) {
	kf_rbtree_foreach(i, &kn_registered_binldrs) {
		kn_binldr_reg_t *binldr = CONTAINER_OF(kn_binldr_reg_t, tree_header, i);

		ps_pcb_t *pcb = kn_alloc_pcb();
		if (!pcb)
			return KM_MAKEERROR(KM_RESULT_NO_MEM);

		if (KM_SUCCEEDED(binldr->binldr.load_exec(pcb, file_handle))) {
			kn_start_user_process(pcb);
		}
	}

	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_EXECFMT);
}

int kn_binldr_reg_keycmp(const kf_rbtree_node_t *x, const void *key) {
	const kn_binldr_reg_t *_x = (const kn_binldr_reg_t *)x;
	const uuid_t *_key = (uuid_t *)key;

	if (uuid_gt(&_x->uuid, _key))
		return 1;
	else if (uuid_lt(&_x->uuid, _key))
		return -1;

	return 0;
}

int kn_binldr_reg_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	const kn_binldr_reg_t *_x = (const kn_binldr_reg_t *)x,
						  *_y = (const kn_binldr_reg_t *)y;

	if (uuid_gt(&_x->uuid, &_y->uuid))
		return 1;
	else if (uuid_lt(&_x->uuid, &_y->uuid))
		return -1;

	return 0;
}

void kn_binldr_reg_nodefree(kf_rbtree_node_t *p) {
	mm_kfree(p);
}
