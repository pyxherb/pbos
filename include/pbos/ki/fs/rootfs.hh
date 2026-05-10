#ifndef _PBOS_KI_FS_ROOTFS_H_
#define _PBOS_KI_FS_ROOTFS_H_

#include "file.hh"
#include "fs.hh"

PBOS_EXTERN_C_BEGIN

extern fs_fsops_t ki_rootfs_ops;
extern fs_file_system_t *fs_rootfs;

km_result_t ki_rootfs_subnode(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
void ki_rootfs_offload(fs_fnode_t *file);
km_result_t ki_rootfs_create_file(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
km_result_t ki_rootfs_create_dir(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
km_result_t ki_rootfs_open(fs_fnode_t *file, fs_fcb_t **fcb_out);
void ki_rootfs_close(fs_fcb_t *fcb);
km_result_t ki_rootfs_read(fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out);
km_result_t ki_rootfs_write(fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out);
km_result_t ki_rootfs_size(fs_fcb_t *fcb, size_t *size_out);
km_result_t ki_rootfs_premount(fs_fnode_t *parent, fs_fnode_t *file);
void ki_rootfs_mountfail(fs_fnode_t *parent, fs_fnode_t *file);
km_result_t ki_rootfs_unmount_cleanup(fs_fnode_t *file);

km_result_t ki_rootfs_destructor();

PBOS_EXTERN_C_END

#endif
