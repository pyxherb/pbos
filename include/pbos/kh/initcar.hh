#ifndef _PBOS_KN_FS_INITCAR_HH_
#define _PBOS_KN_FS_INITCAR_HH_

#include <pbos/fmt/pbcar.h>
#include <pbos/kn/fs/file.hh>
#include <pbos/kn/fs/fs.hh>

PBOS_EXTERN_C_BEGIN

#define INITCAR_UUID UUID(44417a9f, 01be, fd99, 93d2, 010a9fc70042)

typedef struct _kh_initcar_dir_file_t : public fs_fnode_t {
	kf_hashmap_t children;
} kh_initcar_dir_file_t;

typedef struct _kh_initcar_dir_entry_t {
	kf_hashmap_node_t node_header;
	char *name;
	size_t name_len;
	fs_fnode_t *file;
} kh_initcar_dir_entry_t;

typedef struct _kh_initcar_file_t : public fs_fnode_t {
	const char *ptr;
	size_t sz_total;
} kh_initcar_file_t;

km_result_t kh_initcar_subnode(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
void kh_initcar_offload(fs_fnode_t *file);
km_result_t kh_initcar_create_file(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
km_result_t kh_initcar_create_dir(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
km_result_t kh_initcar_open(fs_fnode_t *handle, fs_fcb_t **fcb_out);
void kh_initcar_close(fs_fcb_t *fcb);
km_result_t kh_initcar_read(fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out);
km_result_t kh_initcar_write(fs_fcb_t *fcb, const char *src, size_t size, size_t off, size_t *bytes_written_out);
km_result_t kh_initcar_size(fs_fcb_t *fcb, size_t *size_out);
km_result_t kh_initcar_premount(fs_fnode_t *parent, fs_fnode_t *file);
km_result_t kh_initcar_postmount(fs_fnode_t *parent, fs_fnode_t *file);
void kh_initcar_mount_fail(fs_fnode_t *parent, fs_fnode_t *file);
km_result_t kh_initcar_unmount_cleanup(fs_fnode_t *file);
km_result_t kh_initcar_destructor();

void kh_initcar_init();
void kh_initcar_deinit();

size_t ki_initcar_file_hasher(size_t bucket_num, const void *target, bool is_target_key);
void ki_initcar_file_nodefree(kf_hashmap_node_t *node);
bool ki_initcar_file_nodecmp(const kf_hashmap_node_t *lhs, const kf_hashmap_node_t *rhs);

extern fs_filesys_t *kh_initcar_fs;
extern fs_fnode_t *kh_initcar_dir;
extern fs_fsops_t kh_initcar_ops;

PBOS_EXTERN_C_END

#endif
