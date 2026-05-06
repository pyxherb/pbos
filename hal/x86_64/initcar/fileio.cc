#include <pbos/kf/hash.h>
#include <pbos/km/panic.h>
#include <string.h>
#include <hal/x86_64/initcar.hh>

PBOS_EXTERN_C_BEGIN

hn_initcar_file_keeper_t::hn_initcar_file_keeper_t(fs_fnode_t *file) {
	fs_inc_fnode_ref(file);
	this->rb_value = file;
}
hn_initcar_file_keeper_t::~hn_initcar_file_keeper_t() {
	fs_dec_fnode_ref(rb_value);
}

km_result_t kh_initcar_subnode(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_MAKEERROR(KM_RESULT_NOT_FOUND);
}

void kh_initcar_offload(fs_fnode_t *file) {
}

km_result_t kh_initcar_create_file(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kh_initcar_create_dir(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kh_initcar_open(fs_fnode_t *file, fs_fcb_t **fcb_out) {
	km_result_t result;

	fs::fcb_ptr_t fcb;
	KM_RETURN_IF_FAILED(ki_alloc_fcb(file, fcb.get_addr()));

	*fcb_out = fcb.release();

	return KM_RESULT_OK;
}

void kh_initcar_close(fs_fcb_t *fcb) {
}

km_result_t kh_initcar_read(fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out) {
	km_result_t result;
	fs::fnode_ptr_t file = fs_file_of_fcb(fcb);

	if (auto node = hn_initcar_file_set.find(file.get()); node) {
		hn_initcar_file_keeper_t *exdata = static_cast<hn_initcar_file_keeper_t *>(node);
		if (size + off > exdata->sz_total) {
			memcpy(dest, exdata->ptr + off, exdata->sz_total - off);
			*bytes_read_out = exdata->sz_total - off;
			return KM_MAKEERROR(KM_RESULT_EOF);
		}
		memcpy(dest, exdata->ptr + off, size);
		*bytes_read_out = size;
	} else
		km_panic("Invalid INITCAR FCB");

	return KM_RESULT_OK;
}

km_result_t kh_initcar_write(fs_fcb_t *fcb, const char *src, size_t size, size_t off, size_t *bytes_written_out) {
	*bytes_written_out = 0;
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kh_initcar_size(fs_fcb_t *fcb, size_t *size_out) {
	km_result_t result;
	fs::fnode_ptr_t file = fs_file_of_fcb(fcb);

	if (auto node = hn_initcar_file_set.find(file.get()); node) {
		hn_initcar_file_keeper_t *exdata = static_cast<hn_initcar_file_keeper_t *>(node);
		*size_out - exdata->sz_total;
	} else
		km_panic("Invalid INITCAR FCB");

	return KM_RESULT_OK;
}

PBOS_EXTERN_C_END
