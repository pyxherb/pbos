#ifndef _PBOS_KN_FS_ROOTFS_H_
#define _PBOS_KN_FS_ROOTFS_H_

#include "file.h"
#include "fs.h"

#define ROOTFS_UUID UUID(8ad4b63d, f097, 48e0, 9c68, 77c8606143e9)

extern fs_fsops_t kn_rootfs_ops;
extern fs_filesys_t *fs_rootfs;

/// @brief Extra data for directory files.
typedef struct _fs_rootfs_dir_exdata_t {
	kf_hashmap_t children;
	char exdata[];
} fs_rootfs_dir_exdata_t;

typedef struct _fs_rootfs_dir_entry_t {
	kf_hashmap_node_t node_header;
	char *name;
	size_t name_len;
	fs_file_t *file;
} fs_rootfs_dir_entry_t;

km_result_t kn_rootfs_subnode(fs_file_t *parent, const char *name, size_t name_len, fs_file_t **file_out);
void kn_rootfs_offload(fs_file_t *file);
km_result_t kn_rootfs_create_file(fs_file_t *parent, const char *name, size_t name_len, fs_file_t **file_out);
km_result_t kn_rootfs_create_dir(fs_file_t *parent, const char *name, size_t name_len, fs_file_t **file_out);
km_result_t kn_rootfs_open(fs_file_t *file, fs_fcontext_t **fcontext_out);
km_result_t kn_rootfs_close(fs_fcontext_t *fcontext);
km_result_t kn_rootfs_read(fs_fcontext_t *fcontext, char *dest, size_t size, size_t off, size_t *bytes_read_out);
km_result_t kn_rootfs_write(fs_fcontext_t *fcontext, const char *src, size_t size, size_t off, size_t *bytes_written_out);
km_result_t kn_rootfs_size(fs_fcontext_t *fcontext, size_t *size_out);
km_result_t kn_rootfs_mount(fs_file_t *parent, fs_file_t *file);
km_result_t kn_rootfs_premount(fs_file_t *parent, fs_file_t *file);
km_result_t kn_rootfs_postmount(fs_file_t *parent, fs_file_t *file);
void kn_rootfs_mountfail(fs_file_t *parent, fs_file_t *file);

km_result_t kn_rootfs_destructor();

size_t kn_fs_rootfs_file_hasher(size_t bucket_num, const void *target, bool is_target_key);
void kn_fs_rootfs_file_nodefree(kf_hashmap_node_t *node);
bool kn_fs_rootfs_file_nodecmp(const kf_hashmap_node_t *lhs, const kf_hashmap_node_t *rhs);

#endif
