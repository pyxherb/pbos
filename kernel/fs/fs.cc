#include <pbos/km/logger.h>
#include <pbos/mm/mm.h>
#include <pbos/kn/fs/file.hh>
#include <pbos/kn/fs/fs.hh>
#include <pbos/kn/fs/rootfs.hh>
#include <pbos/kfxx/uuid.hh>

PBOS_EXTERN_C_BEGIN

kfxx::rbtree_t<kf_uuid_t> kn_registered_fs;
fs_fnode_t *fs_abs_root_dir;
fs_filesys_t *fs_rootfs;

_fs_filesys_t::_fs_filesys_t() {
}

_fs_filesys_t::~_fs_filesys_t() {
	if (name)
		kfxx::kernel_allocator()->release(name, name_len, alignof(char));
}

fs_filesys_t *fs_register_filesys(
	const char *name,
	size_t name_len,
	kf_uuid_t *uuid,
	fs_fsops_t *ops) {
	fs_filesys_t *fs = (fs_filesys_t *)kfxx::alloc_and_construct<fs_filesys_t>(kfxx::kernel_allocator());
	if (!fs)
		return nullptr;

	kfxx::scope_guard release_fs_guard([fs]() noexcept {
		kfxx::destroy_and_release<fs_filesys_t>(kfxx::kernel_allocator(), fs);
	});

	if(!(fs->name = (char*)kfxx::kernel_allocator()->alloc(name_len, alignof(char))))
		return nullptr;
	fs->name_len = name_len;
	memcpy(&fs->ops, ops, sizeof(fs_fsops_t));
	memcpy(&fs->rb_value, uuid, sizeof(kf_uuid_t));

	if (!kn_registered_fs.insert(fs))
		return nullptr;

	release_fs_guard.release();

	return fs;
}

void fs_init() {
	km_result_t result;

	kf_uuid_t uuid;

	uuid = ROOTFS_UUID;
	if (!(fs_rootfs = fs_register_filesys("rootfs", strlen("rootfs"), &uuid, &kn_rootfs_ops)))
		km_panic("Error registering root file system");

	kd_printf("Registered root file system\n");

	if (KM_FAILED(result = fs_alloc_dir_fnode(fs_rootfs, &fs_abs_root_dir)))
		km_panic("Error creating the root directory, error code = %.0x", result);

	kd_printf("Created the root directory\n");
}

PBOS_EXTERN_C_END
