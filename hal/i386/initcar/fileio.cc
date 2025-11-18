#include <pbos/kf/hash.h>
#include <pbos/km/panic.h>
#include <hal/i386/initcar.hh>
#include <pbos/km/objmgr.hh>
#include <string.h>

PBOS_EXTERN_C_BEGIN

size_t kn_initcar_file_hasher(size_t bucket_num, const void *target, bool is_target_key) {
	initcar_dir_entry_t *entry = PBOS_CONTAINER_OF(initcar_dir_entry_t, node_header, target);
	fs_file_t *file = entry->file;
	return kf_hash_djb(entry->name, entry->name_len) % bucket_num;
}

void kn_initcar_file_nodefree(kf_hashmap_node_t *node) {
	initcar_dir_entry_t *entry = PBOS_CONTAINER_OF(initcar_dir_entry_t, node_header, node);
	om_decref(entry->file);
}

bool kn_initcar_file_nodecmp(const kf_hashmap_node_t *lhs, const kf_hashmap_node_t *rhs) {
	initcar_dir_entry_t *_lhs = PBOS_CONTAINER_OF(initcar_dir_entry_t, node_header, lhs),
						*_rhs = PBOS_CONTAINER_OF(initcar_dir_entry_t, node_header, rhs);

	fs_file_t *lhs_file = _lhs->file, *rhs_file = _rhs->file;

	uint64_t lhs_hash = kf_hash_djb(_lhs->name, _lhs->name_len),
			 rhs_hash = kf_hash_djb(_rhs->name, _rhs->name_len);

	return lhs_hash == rhs_hash;
}

km_result_t initcar_subnode(fs_file_t *parent, const char *name, size_t name_len, fs_file_t **file_out) {
	if (parent == initcar_dir) {
		initcar_dir_entry_t query_entry = {
			.name = (char *)name,
			.name_len = name_len
		};
		kf_hashmap_node_t *node = kf_hashmap_find(&((initcar_dir_file_t *)parent)->children, &query_entry);
		if (!node)
			return KM_MAKEERROR(KM_RESULT_NOT_FOUND);
		fs_file_t *file = PBOS_CONTAINER_OF(initcar_dir_entry_t, node_header, node)->file;
		om_incref(file);
		*file_out = file;
		return KM_RESULT_OK;
	}
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

void initcar_offload(fs_file_t *file) {
}

km_result_t initcar_create_file(fs_file_t *parent, const char *name, size_t name_len, fs_file_t **file_out) {
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t initcar_create_dir(fs_file_t *parent, const char *name, size_t name_len, fs_file_t **file_out) {
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t initcar_open(fs_file_t *file, fs_fcb_t **fcb_out) {
	km_result_t result;

	fs_fcb_t *fcb = (fs_fcb_t *)mm_kmalloc(sizeof(fs_fcb_t), alignof(fs_fcb_t));
	if (!fcb)
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	memset(fcb, 0, sizeof(fs_fcb_t));
	fcb->file = file;
	om_incref(file);

	*fcb_out = fcb;

	return KM_RESULT_OK;
}

km_result_t initcar_close(fs_fcb_t *fcb) {
	// TODO: Do some checks.
	om_decref(fcb->file);
	mm_kfree(fcb);
	return KM_RESULT_OK;
}

km_result_t initcar_read(fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out) {
	km_result_t result;
	om::object_ptr<fs_file_t> file = fcb->file;

	initcar_file_t *exdata = (initcar_file_t *)file.get();
	if (size + off > exdata->sz_total) {
		memcpy(dest, exdata->ptr + off, exdata->sz_total - off);
		*bytes_read_out = exdata->sz_total - off;
		return KM_MAKEERROR(KM_RESULT_EOF);
	}
	memcpy(dest, exdata->ptr + off, size);
	*bytes_read_out = size;

	return KM_RESULT_OK;
}

km_result_t initcar_write(fs_fcb_t *fcb, const char *src, size_t size, size_t off, size_t *bytes_written_out) {
	*bytes_written_out = 0;
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t initcar_size(fs_fcb_t *fcb, size_t *size_out) {
	km_result_t result;
	om::object_ptr<fs_file_t> file = fcb->file;

	initcar_file_t *exdata = (initcar_file_t *)file.get();
	*size_out = exdata->sz_total;

	return KM_RESULT_OK;
}

PBOS_EXTERN_C_END
