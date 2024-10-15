#include <pbos/kn/fs/initcar.h>
#include <pbos/km/panic.h>
#include <string.h>

km_result_t initcar_open(fs_file_t *file, om_handle_t *handle_out) {
	return om_create_handle(&file->object_header, handle_out);
}

km_result_t initcar_close(om_handle_t handle) {
	return om_close_handle(handle);
}

km_result_t initcar_read(fs_file_t *file, char *dest, size_t size, size_t off, size_t *bytes_read_out) {
	initcar_file_exdata_t *exdata = (initcar_file_exdata_t *)fs_file_exdata(file);
	if (size + off > exdata->sz_total) {
		memcpy(dest, exdata->ptr + off, exdata->sz_total - off);
		*bytes_read_out = exdata->sz_total - off;
		return KM_MAKEERROR(KM_RESULT_EOF);
	}
	memcpy(dest, exdata->ptr + off, size);
	*bytes_read_out = size;
	return KM_RESULT_OK;
}

km_result_t initcar_write(fs_file_t *file, const char *src, size_t size, size_t off, size_t *bytes_written_out) {
	*bytes_written_out = 0;
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t initcar_size(fs_file_t *file, size_t *size_out) {
	initcar_file_exdata_t *exdata = (initcar_file_exdata_t *)file->exdata;
	*size_out = exdata->sz_total;
	return KM_RESULT_OK;
}
