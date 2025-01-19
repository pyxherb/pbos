#include <pbos/km/panic.h>
#include <pbos/kn/fs/initcar.h>
#include <string.h>

km_result_t initcar_open(om_handle_t handle, fs_fcontext_t **fcontext_out) {
	km_result_t result;
	fs_file_t *file;

	fs_fcontext_t *fcontext = mm_kmalloc(sizeof(fs_fcontext_t));
	if (!fcontext)
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	om_init_object(&fcontext->object_header, fs_fcontext_class);
	fcontext->filesys = initcar_fs;
	fcontext->file_handle = handle;
	om_ref_handle(handle);

	*fcontext_out = fcontext;

	return KM_RESULT_OK;
}

void initcar_close(fs_fcontext_t *fcontext) {
	om_close_handle(fcontext->file_handle);
	mm_kfree(fcontext);
}

km_result_t initcar_read(fs_fcontext_t *fcontext, char *dest, size_t size, size_t off, size_t *bytes_read_out) {
	km_result_t result;
	fs_file_t *file = NULL;

	if (KM_FAILED(result = fs_deref_file_handle(fcontext->file_handle, &file)))
		goto cleanup;

	initcar_file_exdata_t *exdata = (initcar_file_exdata_t *)fs_file_exdata(file);
	if (size + off > exdata->sz_total) {
		memcpy(dest, exdata->ptr + off, exdata->sz_total - off);
		*bytes_read_out = exdata->sz_total - off;
		result = KM_MAKEERROR(KM_RESULT_EOF);
		goto cleanup;
	}
	memcpy(dest, exdata->ptr + off, size);
	*bytes_read_out = size;
	result = KM_RESULT_OK;

cleanup:
	if (file) {
		om_decref(&file->object_header);
	}
	return result;
}

km_result_t initcar_write(fs_fcontext_t *fcontext, const char *src, size_t size, size_t off, size_t *bytes_written_out) {
	*bytes_written_out = 0;
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t initcar_size(fs_fcontext_t *fcontext, size_t *size_out) {
	km_result_t result;
	fs_file_t *file = NULL;

	if (KM_FAILED(result = fs_deref_file_handle(fcontext->file_handle, &file)))
		goto cleanup;

	initcar_file_exdata_t *exdata = (initcar_file_exdata_t *)file->exdata;
	*size_out = exdata->sz_total;
	result = KM_RESULT_OK;

cleanup:
	if (file) {
		om_decref(&file->object_header);
	}
	return result;
}
