#include <pbos/km/logger.h>
#include <pbos/mm/mm.h>
#include <pbos/kn/fs/rootfs.hh>
#include <pbos/kn/fs/file.hh>
#include <pbos/kn/fs/fs.hh>

PBOS_EXTERN_C_BEGIN

kfxx::rbtree_t<kf_uuid_t> kn_registered_fs;
fs_fnode_t *fs_abs_root_dir;
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

	kf_uuid_t uuid;

	uuid = ROOTFS_UUID;
	if (!(fs_rootfs = fs_register_filesys("rootfs", strlen("rootfs"), &uuid, &kn_rootfs_ops)))
		km_panic("Error registering root file system");

	kd_printf("Registered root file system\n");

	if (KM_FAILED(result = kn_alloc_dir_fnode(&fs_abs_root_dir)))
		km_panic("Error creating the root directory, error code = %.0x", result);

	kd_printf("Created the root directory\n");
}

PBOS_EXTERN_C_END
