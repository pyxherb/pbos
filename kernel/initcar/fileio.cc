#include <pbos/kh/initcar.hh>
#include <pbos/kf/hash.h>
#include <pbos/km/panic.h>
#include <string.h>

PBOS_EXTERN_C_BEGIN

km_result_t kh_initcar_subnode(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_RESULT_NOT_FOUND;
}

void kh_initcar_offload(fs_fnode_t *file) {
}

km_result_t kh_initcar_create_file(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

km_result_t kh_initcar_create_dir(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

km_result_t kh_initcar_open(fs_fnode_t *file, fs_fcb_t **fcb_out) {
	km_result_t result;

	return fs_create_fcb(file, fcb_out);
}

void kh_initcar_close(fs_fcb_t *fcb) {
}

km_result_t kh_initcar_read(fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out) {
	km_result_t result;
	fs::fnode_ptr file = fs_file_of_fcb(fcb);

	auto exdata = (kh_initcar_file_exdata *)fs_get_fnode_exdata(file.get());

	if (size + off > exdata->sz_total) {
		memcpy(dest, exdata->ptr + off, exdata->sz_total - off);
		*bytes_read_out = exdata->sz_total - off;
		return KM_RESULT_EOF;
	}
	memcpy(dest, exdata->ptr + off, size);
	*bytes_read_out = size;

	return KM_RESULT_OK;
}

km_result_t kh_initcar_write(fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out) {
	*bytes_written_out = 0;
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

km_result_t kh_initcar_size(fs_fcb_t *fcb, size_t *size_out) {
	km_result_t result;
	fs::fnode_ptr file = fs_file_of_fcb(fcb);

	auto exdata = (kh_initcar_file_exdata *)fs_get_fnode_exdata(file.get());

	*size_out = exdata->sz_total;

	return KM_RESULT_OK;
}

km_result_t kh_initcar_enum_first_child_file(fs_fnode_t *dir, fs_fnode_t **first_file_out) {
	if(dir != kh_initcar_dir) {
		*first_file_out = nullptr;
		return KM_RESULT_UNSUPPORTED_OPERATION;
	}
	*first_file_out = kh_initcar_first_file;
	return KM_RESULT_OK;
}

km_result_t kh_initcar_enum_next_file(fs_fnode_t *cur_file, fs_fnode_t **next_file_out) {
	if(!cur_file) {
		*next_file_out = nullptr;
		return KM_RESULT_INVALID_ARGS;
	}
	*next_file_out = ((kh_initcar_file_exdata *)fs_get_fnode_exdata(cur_file))->next;
	return KM_RESULT_OK;
}

PBOS_EXTERN_C_END
