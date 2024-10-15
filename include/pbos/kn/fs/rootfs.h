#ifndef _PBOS_KN_FS_ROOTFS_H_
#define _PBOS_KN_FS_ROOTFS_H_

#include "fs.h"
#include "file.h"

#define ROOTFS_UUID UUID(8ad4b63d, f097, 48e0, 9c68, 77c8606143e9)

extern fs_fsops_t kn_rootfs_ops;

typedef struct _kn_rootfs_fcb_exdata_t {
	om_handle_t file_handle;
} kn_rootfs_fcb_exdata_t;

km_result_t kn_rootfs_open(fs_file_t *file, om_handle_t* handle_out);
km_result_t kn_rootfs_close(om_handle_t handle);
km_result_t kn_rootfs_read(fs_file_t *file, char *dest, size_t size, size_t off, size_t *bytes_read_out);
km_result_t kn_rootfs_write(fs_file_t *file, const char *src, size_t size, size_t off, size_t *bytes_written_out);
km_result_t kn_rootfs_size(fs_file_t *file, size_t *size_out);
km_result_t kn_rootfs_premount(fs_file_t * parent, fs_file_t * file_handle);
km_result_t kn_rootfs_postmount(fs_file_t * parent, fs_file_t * file_handle);
void kn_rootfs_mountfail(fs_file_t * parent, fs_file_t * file_handle);

km_result_t kn_rootfs_destructor();


#endif
