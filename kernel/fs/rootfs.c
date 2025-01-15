#include <pbos/km/mm.h>
#include <pbos/kn/fs/rootfs.h>

fs_fsops_t kn_rootfs_ops = {
	.open = kn_rootfs_open,
	.close = kn_rootfs_close,
	.read = kn_rootfs_read,
	.write = kn_rootfs_write,
	.size = kn_rootfs_size,
	.premount = kn_rootfs_premount,
	.postmount = kn_rootfs_postmount,
	.mountfail = kn_rootfs_mountfail,
	.destructor = kn_rootfs_destructor
};

km_result_t kn_rootfs_open(om_handle_t file_handle, fs_fcontext_t **fcontext_out) {
	km_result_t result;
	fs_file_t *file;

	fs_fcontext_t *fcontext = mm_kmalloc(sizeof(fs_fcontext_t));
	if (!fcontext)
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	fcontext->filesys = fs_rootfs;
	fcontext->file_handle = file_handle;
	om_ref_handle(file_handle);

	*fcontext_out = fcontext;

	return KM_RESULT_OK;
}

km_result_t kn_rootfs_close(fs_fcontext_t *fcontext) {
	om_close_handle(fcontext->file_handle);
	mm_kfree(fcontext);
	return KM_RESULT_OK;
}

km_result_t kn_rootfs_read(fs_fcontext_t *fcontext, char *dest, size_t size, size_t off, size_t *bytes_read_out) {
	*bytes_read_out = 0;
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kn_rootfs_write(fs_fcontext_t *fcontext, const char *src, size_t size, size_t off, size_t *bytes_written_out) {
	*bytes_written_out = 0;
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kn_rootfs_size(fs_fcontext_t *fcontext, size_t *size_out) {
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kn_rootfs_premount(om_handle_t parent, om_handle_t file_handle) {
	return KM_RESULT_OK;
}

km_result_t kn_rootfs_postmount(om_handle_t parent, om_handle_t file_handle) {
	return KM_RESULT_OK;
}

void kn_rootfs_mountfail(om_handle_t parent, om_handle_t file_handle) {
}

km_result_t kn_rootfs_destructor() {
	return KM_RESULT_OK;
}
