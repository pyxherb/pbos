#ifndef _PBOS_KI_FS_DEVICEFS_H_
#define _PBOS_KI_FS_DEVICEFS_H_

#include "file.hh"
#include "fs.hh"
#include <pbos/ki/dm/device.h>

PBOS_EXTERN_C_BEGIN

struct ki_devio_fnode_exdata_t {
	fs_fnode_t *prev = nullptr, *next = nullptr;
	ps::semaphore_t critical_semaphore;
};

struct ki_devio_dir_exdata_t : public ki_devio_fnode_exdata_t {
	fs_fnode_t *first_child = nullptr;
};

struct ki_devio_file_exdata_t : public ki_devio_fnode_exdata_t {
	dm_device_t *device = nullptr;
};

extern fs_filesys_ops_t ki_devio_ops;
extern fs_filesys_t *ki_devio_filesys;
extern fs::fnode_ptr ki_devio_root_dir;

constexpr kfxx::string_view KI_DEVIO_ROOT_DIR_NAME = "dev";

km_result_t ki_devio_subnode(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);

void ki_devio_offload(fs_fnode_t *file);

km_result_t ki_devio_create_file(io_dispatch_context_t *dc, fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
km_result_t ki_devio_create_dir(io_dispatch_context_t *dc, fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);

km_result_t ki_devio_remove_file(io_dispatch_context_t *dc, fs_fnode_t *fnode);

km_result_t ki_devio_open(fs_fnode_t *file, fs_fcb_t **fcb_out, fs_open_flags_t flags);
void ki_devio_close(fs_fcb_t *fcb);

km_result_t ki_devio_seek(io_dispatch_context_t *dc, fs_fcb_t *fcb, long off, fs_seek_mode_t mode);
km_result_t ki_devio_read(io_dispatch_context_t *dc, fs_fcb_t *fcb, char *dest, size_t size, size_t *bytes_read_out);
km_result_t ki_devio_write(io_dispatch_context_t *dc, fs_fcb_t *fcb, const void *src, size_t size, size_t *bytes_written_out);

km_result_t ki_devio_pread(io_dispatch_context_t *dc, fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out);
km_result_t ki_devio_pwrite(io_dispatch_context_t *dc, fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out);

km_result_t ki_devio_ioctl(io_dispatch_context_t *dc, fs_fcb_t *fcb, uint32_t ioctl_code, void *data_in, size_t size_in, void *data_out, size_t size_out, void *args);

km_result_t ki_devio_size(io_dispatch_context_t *dc, fs_fcb_t *fcb, size_t *size_out);

void ki_devio_destroy(fs_fnode_t *file);

km_result_t ki_devio_premount(fs_fnode_t *parent, fs_fnode_t *file);
void ki_devio_mountfail(fs_fnode_t *parent, fs_fnode_t *file);

km_result_t ki_devio_unmount_cleanup(fs_fnode_t *file);

km_result_t ki_devio_destructor();

void ki_devio_init();
void ki_devio_deinit();

PBOS_EXTERN_C_END

#endif
