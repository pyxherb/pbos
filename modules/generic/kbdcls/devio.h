#ifndef _KBDCLS_DEVIO_H_
#define _KBDCLS_DEVIO_H_

#include "common.h"

PBOS_EXTERN_C_BEGIN

km_result_t kbdcls_devio_open(dm_device_t *device, fs_fnode_t *fnode, fs_fcb_t **fcb_out, fs_open_flags_t flags);
void kbdcls_devio_close_cleanup(fs_fcb_t *fcb);

km_result_t kbdcls_devio_remove(io_dispatch_context_t *dc, dm_device_t *device, fs_fnode_t *fnode);

km_result_t kbdcls_devio_read(io_dispatch_context_t *dc, fs_fcb_t *fcb, void *dest, size_t size, size_t *bytes_read_out);
km_result_t kbdcls_devio_write(io_dispatch_context_t *dc, fs_fcb_t *fcb, const void *src, size_t size, size_t *bytes_written_out);

km_result_t kbdcls_devio_pread(io_dispatch_context_t *dc, fs_fcb_t *fcb, void *dest, size_t size, size_t off, size_t *bytes_read_out);
km_result_t kbdcls_devio_pwrite(io_dispatch_context_t *dc, fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out);

km_result_t kbdcls_devio_ioctl(io_dispatch_context_t *dc, fs_fcb_t *fcb, uint32_t ioctl_code, void *data_in, size_t size_in, void *data_out, size_t size_out, void *args);

PBOS_EXTERN_C_END

#endif
