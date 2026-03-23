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
	.mount = kn_rootfs_mount,
	.premount = kn_rootfs_premount,
	.postmount = kn_rootfs_postmount,
	.mountfail = kn_rootfs_mountfail,
	.destructor = kn_rootfs_destructor
};

km_result_t kn_rootfs_subnode(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	km_result_t result;

	switch (parent->filetype) {
		case FS_FILETYPE_DIR: {
			// Copy the file name.
			fs_rootfs_dir_entry_t k = {
				.name = (char *)name,
				.name_len = name_len
			};

			kf_hashmap_node_t *node = kf_hashmap_find(&((fs_rootfs_dir_t *)(parent))->children, &k);
			if (!node) {
				result = KM_MAKEERROR(KM_RESULT_NOT_FOUND);
				goto cleanup;
			}

			om::object_ptr<fs_fnode_t> file = PBOS_CONTAINER_OF(fs_rootfs_dir_entry_t, node_header, node)->file;

			*file_out = file.release();
			break;
		}
		case FS_FILETYPE_LINK:
			// stub
		default:
			return KM_MAKEERROR(KM_RESULT_NOT_FOUND);
	}

	result = KM_RESULT_OK;

cleanup:
	return result;
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

km_result_t kn_rootfs_close(fs_fcb_t *fcb) {
	// TODO: Do some checks.
	om_decref(fcb->file);
	mm_kfree(fcb);
	return KM_RESULT_OK;
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

km_result_t kn_rootfs_mount(fs_fnode_t *parent, fs_fnode_t *file) {
	fs_rootfs_dir_entry_t *dir_entry = (fs_rootfs_dir_entry_t *)mm_kmalloc(sizeof(fs_rootfs_dir_entry_t), alignof(fs_rootfs_dir_entry_t));
	if (!dir_entry)
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	memset(dir_entry, 0, sizeof(*dir_entry));
	dir_entry->name = file->filename;
	dir_entry->name_len = file->filename_len;
	dir_entry->file = file;
	km_result_t result = kf_hashmap_insert(&((fs_rootfs_dir_t *)parent)->children, &dir_entry->node_header);
	kd_assert(KM_SUCCEEDED(result));
	return KM_RESULT_OK;
}

km_result_t kn_rootfs_premount(fs_fnode_t *parent, fs_fnode_t *file) {
	return KM_RESULT_OK;
}

km_result_t kn_rootfs_postmount(fs_fnode_t *parent, fs_fnode_t *file) {
	return KM_RESULT_OK;
}

void kn_rootfs_mountfail(fs_fnode_t *parent, fs_fnode_t *file) {
}

km_result_t kn_rootfs_destructor() {
	return KM_RESULT_OK;
}
