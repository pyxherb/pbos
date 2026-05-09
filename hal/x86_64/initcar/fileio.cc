#include <pbos/kf/hash.h>
#include <pbos/km/panic.h>
#include <string.h>
#include <hal/x86_64/initcar.hh>

PBOS_EXTERN_C_BEGIN

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

	return ki_alloc_fcb(file, fcb_out);
}

void kh_initcar_close(fs_fcb_t *fcb) {
}

km_result_t kh_initcar_read(fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out) {
	km_result_t result;
	fs::fnode_ptr_t file = fs_file_of_fcb(fcb);

	auto exdata = (hn_initcar_file_exdata *)fs_get_fnode_exdata(file.get());

	if (size + off > exdata->sz_total) {
		memcpy(dest, exdata->ptr + off, exdata->sz_total - off);
		*bytes_read_out = exdata->sz_total - off;
		return KM_MAKEERROR(KM_RESULT_EOF);
	}
	memcpy(dest, exdata->ptr + off, size);
	*bytes_read_out = size;

	return KM_RESULT_OK;
}

km_result_t kh_initcar_write(fs_fcb_t *fcb, const char *src, size_t size, size_t off, size_t *bytes_written_out) {
	*bytes_written_out = 0;
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kh_initcar_size(fs_fcb_t *fcb, size_t *size_out) {
	km_result_t result;
	fs::fnode_ptr_t file = fs_file_of_fcb(fcb);

	auto exdata = (hn_initcar_file_exdata *)fs_get_fnode_exdata(file.get());

	*size_out = exdata->sz_total;

	return KM_RESULT_OK;
}

PBOS_EXTERN_C_END
