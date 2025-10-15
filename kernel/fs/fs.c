#include <pbos/kf/string.h>
#include <pbos/km/logger.h>
#include <pbos/km/mm.h>
#include <pbos/kn/fs/file.h>
#include <pbos/kn/fs/fs.h>
#include <pbos/kn/fs/rootfs.h>

static bool _filesys_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
static void _filesys_nodefree(kf_rbtree_node_t *p);

kf_rbtree_t kn_registered_fs;
fs_file_t *fs_abs_root_dir;
om_class_t *fs_file_class;
om_class_t *fs_fcb_class;
fs_filesys_t *fs_rootfs;

fs_filesys_t *fs_register_filesys(
	const char *name,
	uuid_t *uuid,
	fs_fsops_t *ops) {
	fs_filesys_t *fs = mm_kmalloc(sizeof(fs_filesys_t));
	if (!fs)
		return NULL;

	memset(fs, 0, sizeof(fs_filesys_t));
	strncpy(fs->name, name, sizeof(fs->name));
	memcpy(&fs->uuid, uuid, sizeof(uuid_t));
	memcpy(&fs->ops, ops, sizeof(fs_fsops_t));

	kf_rbtree_insert(&kn_registered_fs, &fs->tree_header);

	return fs;
}

void fs_init() {
	km_result_t result;

	kf_rbtree_init(
		&kn_registered_fs,
		_filesys_nodecmp,
		_filesys_nodefree);

	uuid_t uuid = FILE_CLASS_UUID;
	if (!(fs_file_class = om_register_class(&uuid, kn_file_destructor)))
		km_panic("Error registering file class");

	uuid = UUID(ffffffff, ffff, ffff, ffff, ffffffff);
	if (!(fs_rootfs = fs_register_filesys("rootfs", &uuid, &kn_rootfs_ops)))
		km_panic("Error registering root file system");

	kdprintf("Registered root file system\n");

	if (KM_FAILED(result = kn_alloc_file(fs_rootfs, "/", sizeof("/") - 1, FS_FILETYPE_DIR, sizeof(fs_rootfs_dir_exdata_t), &fs_abs_root_dir)))
		km_panic("Error creating the root directory, error code = %.0x", result);

	kf_hashmap_init(&((fs_rootfs_dir_exdata_t *)fs_abs_root_dir->exdata)->children, kn_fs_rootfs_file_hasher, kn_fs_rootfs_file_nodefree, kn_fs_rootfs_file_nodecmp, NULL);

	kdprintf("Created the root directory\n");
}

static bool _filesys_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	fs_filesys_t *_x = PBOS_CONTAINER_OF(fs_filesys_t, tree_header, x),
				 *_y = PBOS_CONTAINER_OF(fs_filesys_t, tree_header, y);

	return uuid_lt(&_x->uuid, &_y->uuid);
}

static void _filesys_nodefree(kf_rbtree_node_t *p) {
	PBOS_CONTAINER_OF(fs_filesys_t, tree_header, p)->ops.destructor();
}
