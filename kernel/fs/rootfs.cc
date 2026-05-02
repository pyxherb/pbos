#include <pbos/kf/hash.h>
#include <pbos/mm/mm.h>
#include <string.h>
#include <pbos/kn/fs/rootfs.hh>

fs_fsops_t kn_rootfs_ops = {
	.subnode = kn_rootfs_subnode,
	.offload = kn_rootfs_offload,
	.create_file = kn_rootfs_create_file,
	.open = kn_rootfs_open,
	.close = kn_rootfs_close,
	.read = kn_rootfs_read,
	.write = kn_rootfs_write,
	.size = kn_rootfs_size,
	.premount = kn_rootfs_premount,
	.mount_fail = kn_rootfs_mountfail,
	.unmount_cleanup = kn_rootfs_unmount_cleanup,
	.destructor = kn_rootfs_destructor
};

km_result_t kn_rootfs_subnode(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_RESULT_NOT_FOUND;
}

void kn_rootfs_offload(fs_fnode_t *file) {
}

km_result_t kn_rootfs_create_file(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kn_rootfs_create_dir(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kn_rootfs_open(fs_fnode_t *file, fs_fcb_t **fcb_out) {
	KM_RETURN_IF_FAILED(fs_create_fcb(file, fcb_out));

	return KM_RESULT_OK;
}

void kn_rootfs_close(fs_fcb_t *fcb) {
}

km_result_t kn_rootfs_read(fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out) {
	*bytes_read_out = 0;
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kn_rootfs_write(fs_fcb_t *fcb, const char *src, size_t size, size_t off, size_t *bytes_written_out) {
	*bytes_written_out = 0;
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kn_rootfs_size(fs_fcb_t *fcb, size_t *size_out) {
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t kn_rootfs_premount(fs_fnode_t *parent, fs_fnode_t *file) {
	return KM_RESULT_OK;
}

km_result_t kn_rootfs_unmount_cleanup(fs_fnode_t *file) {
	return KM_RESULT_OK;
}

void kn_rootfs_mountfail(fs_fnode_t *parent, fs_fnode_t *file) {
}

km_result_t kn_rootfs_destructor() {
	return KM_RESULT_OK;
}
