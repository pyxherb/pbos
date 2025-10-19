#include <pbos/km/logger.h>
#include <pbos/km/mm.h>
#include <pbos/kn/fs/rootfs.hh>
#include <pbos/kn/fs/file.hh>
#include <pbos/kn/fs/fs.hh>

PBOS_EXTERN_C_BEGIN

kfxx::rbtree_t<kf_uuid_t> kn_registered_fs;
fs_file_t *fs_abs_root_dir;
om_class_t *fs_file_class;
om_class_t *fs_fcb_class;
fs_filesys_t *fs_rootfs;

fs_filesys_t *fs_register_filesys(
	const char *name,
	size_t name_len,
	kf_uuid_t *uuid,
	fs_fsops_t *ops) {
	fs_filesys_t *fs = (fs_filesys_t *)mm_kmalloc(sizeof(fs_filesys_t), alignof(fs_filesys_t));
	if (!fs)
		return NULL;

	fs->name = kfxx::string_view(name, name_len);
	fs->rb_value = *uuid;
	memcpy(&fs->ops, ops, sizeof(fs_fsops_t));

	kn_registered_fs.insert(fs);

	return fs;
}

void fs_init() {
	km_result_t result;

	kf_uuid_t uuid = FILE_CLASS_UUID;
	if (!(fs_file_class = om_register_class(&uuid, kn_file_destructor)))
		km_panic("Error registering file class");

	uuid = ROOTFS_UUID;
	if (!(fs_rootfs = fs_register_filesys("rootfs", strlen("rootfs"), &uuid, &kn_rootfs_ops)))
		km_panic("Error registering root file system");

	kdprintf("Registered root file system\n");

	if (KM_FAILED(result = kn_alloc_file(fs_rootfs, "/", strlen("/"), FS_FILETYPE_DIR, sizeof(fs_rootfs_dir_exdata_t), &fs_abs_root_dir)))
		km_panic("Error creating the root directory, error code = %.0x", result);

	kf_hashmap_init(&((fs_rootfs_dir_exdata_t *)fs_abs_root_dir->exdata)->children, kn_fs_rootfs_file_hasher, kn_fs_rootfs_file_nodefree, kn_fs_rootfs_file_nodecmp, NULL);

	kdprintf("Created the root directory\n");
}

PBOS_EXTERN_C_END
