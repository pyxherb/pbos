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

km_result_t kn_rootfs_open(fs_file_t *file, om_handle_t *handle_out) {
	return om_create_handle(&file->object_header, handle_out);
}

km_result_t kn_rootfs_close(om_handle_t handle) {
	return om_close_handle(handle);
}

km_result_t kn_rootfs_read(fs_file_t *file, char *dest, size_t size, size_t off, size_t *bytes_read_out) {
	*bytes_read_out = 0;
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kn_rootfs_write(fs_file_t *file, const char *src, size_t size, size_t off, size_t *bytes_written_out) {
	*bytes_written_out = 0;
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kn_rootfs_size(fs_file_t *file, size_t *size_out) {
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kn_rootfs_premount(fs_file_t *parent, fs_file_t *file_handle) {
	return KM_RESULT_OK;
}

km_result_t kn_rootfs_postmount(fs_file_t *parent, fs_file_t *file_handle) {
	return KM_RESULT_OK;
}

void kn_rootfs_mountfail(fs_file_t *parent, fs_file_t *file_handle) {
}

km_result_t kn_rootfs_destructor() {
	return KM_RESULT_OK;
}
