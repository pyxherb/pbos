#include <pbos/kd/logger.h>
#include <pbos/mm/mm.h>
#include <pbos/kfxx/uuid.hh>
#include <pbos/ki/fs/file.hh>
#include <pbos/ki/fs/fs.hh>
#include <pbos/ki/fs/rootfs.hh>
#include <pbos/ki/km/symbol.hh>

PBOS_EXTERN_C_BEGIN

kfxx::hashmap_t<kfxx::string_view, fs_filesys_t *> ki_registered_fs(kfxx::kernel_allocator());
fs_fnode_t *fs_abs_root_dir;
fs_filesys_t *fs_rootfs;

_fs_filesys_t::_fs_filesys_t() {
}

_fs_filesys_t::~_fs_filesys_t() {
	if (name)
		kfxx::kernel_allocator()->release(name, name_len, alignof(char));
}

PBOS_API fs_filesys_t *fs_register_file_system(
	const char *name,
	size_t name_len,
	fs_filesys_ops_t *ops,
	fs_filesys_t **fs_out) {
	fs_filesys_t *fs = (fs_filesys_t *)kfxx::alloc_and_construct<fs_filesys_t>(kfxx::kernel_allocator());
	if (!fs)
		return nullptr;

	kfxx::scope_guard release_fs_guard([fs]() noexcept {
		kfxx::destroy_and_release<fs_filesys_t>(kfxx::kernel_allocator(), fs);
	});

	if (!(fs->name = (char *)kfxx::kernel_allocator()->alloc(name_len, alignof(char))))
		return nullptr;
	fs->name_len = name_len;
	memcpy(&fs->ops, ops, sizeof(fs_filesys_ops_t));

	if (!ki_registered_fs.insert(kfxx::string_view(fs->name, name_len), +fs))
		return nullptr;

	release_fs_guard.release();

	if(fs_out)
		*fs_out = fs;

	return fs;
}

void ki_fs_init() {
	km_result_t result;

	if (!(fs_rootfs = fs_register_file_system("rootfs", strlen("rootfs"), &ki_rootfs_ops, nullptr)))
		km_panic("Error registering root file system");

	if (KM_FAILED(result = fs_alloc_dir_fnode(fs_rootfs, &fs_abs_root_dir)))
		km_panic("Error creating the root directory, error code = %.0x", result);
}

PBOS_EXTERN_C_END
