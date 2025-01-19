#ifndef _PBOS_KN_FS_ROOTFS_H_
#define _PBOS_KN_FS_ROOTFS_H_

#include "fs.h"
#include "file.h"

#define ROOTFS_UUID UUID(8ad4b63d, f097, 48e0, 9c68, 77c8606143e9)

extern fs_fsops_t kn_rootfs_ops;
extern fs_filesys_t *fs_rootfs;

km_result_t kn_rootfs_open(om_handle_t file, fs_fcontext_t **fcontext_out);
void kn_rootfs_close(fs_fcontext_t *fcontext);
km_result_t kn_rootfs_read(fs_fcontext_t *fcontext, char *dest, size_t size, size_t off, size_t *bytes_read_out);
km_result_t kn_rootfs_write(fs_fcontext_t *fcontext, const char *src, size_t size, size_t off, size_t *bytes_written_out);
km_result_t kn_rootfs_size(fs_fcontext_t *fcontext, size_t *size_out);
km_result_t kn_rootfs_premount(om_handle_t parent, om_handle_t file_handle);
km_result_t kn_rootfs_postmount(om_handle_t parent, om_handle_t file_handle);
void kn_rootfs_mountfail(om_handle_t parent, om_handle_t file_handle);

km_result_t kn_rootfs_destructor();


#endif
