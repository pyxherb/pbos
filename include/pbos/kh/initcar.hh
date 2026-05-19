#ifndef _PBOS_KI_FS_INITCAR_HH_
#define _PBOS_KI_FS_INITCAR_HH_

#include <pbos/fmt/pbcar.h>
#include <pbos/ki/fs/file.hh>
#include <pbos/ki/fs/fs.hh>

PBOS_EXTERN_C_BEGIN

struct kh_initcar_file_exdata {
	fs_fnode_t *prev = nullptr, *next = nullptr;
	fs_file_t *file = nullptr;
	const char *ptr = nullptr;
	size_t sz_total = 0;
};

extern void *kh_initcar_ptr;
extern void *kh_initcar_paddr;
extern size_t kh_initcar_file_size;
extern bool kh_is_initcar_direct_mapped;
extern fs_fnode_t *kh_initcar_first_file, *kh_initcar_last_file;

km_result_t kh_initcar_subnode(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
void kh_initcar_offload(fs_fnode_t *file);

km_result_t kh_initcar_create_file(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
km_result_t kh_initcar_create_dir(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);

km_result_t kh_initcar_open(fs_fnode_t *handle, fs_fcb_t **fcb_out);
void kh_initcar_close(fs_fcb_t *fcb);

km_result_t kh_initcar_read(fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out);
km_result_t kh_initcar_write(fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out);

km_result_t kh_initcar_size(fs_fcb_t *fcb, size_t *size_out);

km_result_t kh_initcar_enum_first_child_file(fs_fnode_t *dir, fs_fnode_t **first_file_out);
km_result_t kh_initcar_enum_next_file(fs_fnode_t *cur_file, fs_fnode_t **next_file_out);

void kh_initcar_destroy(fs_fnode_t *file);

km_result_t kh_initcar_premount(fs_fnode_t *parent, fs_fnode_t *file);
km_result_t kh_initcar_postmount(fs_fnode_t *parent, fs_fnode_t *file);
void kh_initcar_mount_fail(fs_fnode_t *parent, fs_fnode_t *file);
km_result_t kh_initcar_unmount_cleanup(fs_fnode_t *file);

km_result_t kh_initcar_destructor();

void kh_initcar_init();
void kh_initcar_deinit();

extern fs_file_system_t *kh_initcar_fs;
extern fs_fnode_t *kh_initcar_dir;
extern fs_file_system_ops_t kh_initcar_ops;

PBOS_EXTERN_C_END

#endif
