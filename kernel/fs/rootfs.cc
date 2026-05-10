#include <pbos/kf/hash.h>
#include <pbos/mm/mm.h>
#include <string.h>
#include <pbos/ki/fs/rootfs.hh>

fs_fsops_t ki_rootfs_ops = {
	.subnode = ki_rootfs_subnode,
	.offload = ki_rootfs_offload,
	.create_file = ki_rootfs_create_file,
	.open = ki_rootfs_open,
	.close = ki_rootfs_close,
	.read = ki_rootfs_read,
	.write = ki_rootfs_write,
	.size = ki_rootfs_size,
	.premount = ki_rootfs_premount,
	.mount_fail = ki_rootfs_mountfail,
	.unmount_cleanup = ki_rootfs_unmount_cleanup,
	.destructor = ki_rootfs_destructor
};

km_result_t ki_rootfs_subnode(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_RESULT_NOT_FOUND;
}

void ki_rootfs_offload(fs_fnode_t *file) {
}

km_result_t ki_rootfs_create_file(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

km_result_t ki_rootfs_create_dir(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

km_result_t ki_rootfs_open(fs_fnode_t *file, fs_fcb_t **fcb_out) {
	KM_RETURN_IF_FAILED(fs_create_fcb(file, fcb_out));

	return KM_RESULT_OK;
}

void ki_rootfs_close(fs_fcb_t *fcb) {
}

km_result_t ki_rootfs_read(fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out) {
	*bytes_read_out = 0;
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

km_result_t ki_rootfs_write(fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out) {
	*bytes_written_out = 0;
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

km_result_t ki_rootfs_size(fs_fcb_t *fcb, size_t *size_out) {
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

km_result_t ki_rootfs_premount(fs_fnode_t *parent, fs_fnode_t *file) {
	return KM_RESULT_OK;
}

km_result_t ki_rootfs_unmount_cleanup(fs_fnode_t *file) {
	return KM_RESULT_OK;
}

void ki_rootfs_mountfail(fs_fnode_t *parent, fs_fnode_t *file) {
}

km_result_t ki_rootfs_destructor() {
	return KM_RESULT_OK;
}
