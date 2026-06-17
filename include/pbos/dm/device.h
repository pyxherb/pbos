#ifndef _PBOS_DM_DEVICE_H_
#define _PBOS_DM_DEVICE_H_

#include <pbos/kf/uuid.h>
#include <pbos/km/result.h>
#include <pbos/fs/file.h>

PBOS_EXTERN_C_BEGIN

typedef struct _dm_device_t dm_device_t;
typedef struct _dm_bus_t dm_bus_t;

typedef struct _dm_device_class_t dm_device_class_t;

#define DM_DEVICE_QUERY_MODE_SINGLE_ID KF_UUID(bad7a865, 758c, 40fb, 8a86, 5af9ccf2325f)

typedef km_result_t (*dm_device_open_op_t)(dm_device_t *file, fs_fcb_t **fcb_out, fs_open_flags_t flags);
typedef void (*dm_device_close_op_t)(fs_fcb_t *fcb);

typedef km_result_t (*dm_device_remove_t)(io_dispatch_context_t *dc, dm_device_t *device);

typedef km_result_t (*dm_device_seek_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, long off, fs_seek_mode_t mode);
typedef km_result_t (*dm_device_read_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, char *dest, size_t size, size_t *bytes_read_out);
typedef km_result_t (*dm_device_write_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, const void *src, size_t size, size_t *bytes_written_out);

typedef km_result_t (*dm_device_pread_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out);
typedef km_result_t (*dm_device_pwrite_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out);

typedef km_result_t (*ki_devio_ioctl_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, uint32_t ioctl_code, void *data_in, size_t size_in, void *data_out, size_t size_out, void *args);

typedef km_result_t (*dm_device_size_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, size_t *size_out);

typedef km_result_t (*dm_device_query_op_t)(dm_device_t *device, dm_device_t **subdevice_out, kf_uuid_t *mode, void *args);

typedef km_result_t (*dm_device_prelink_op_t)(dm_device_t *parent, dm_device_t *device);

typedef void (*dm_device_unlink_cleanup_op_t)(dm_device_t *parent, dm_device_t *device);

typedef void (*dm_device_destroy_op_t)(dm_device_t *device);

typedef struct _dm_device_ops_t {
	dm_device_open_op_t open;
	/// @brief Close a FCB.
	dm_device_close_op_t close;

	dm_device_remove_t remove;

	dm_device_seek_op_t seek;
	dm_device_read_op_t read;
	dm_device_write_op_t write;

	/// @brief Read data from a file.
	dm_device_pread_op_t pread;
	/// @brief Write data into a file.
	dm_device_pwrite_op_t pwrite;

	ki_devio_ioctl_op_t ioctl;

	dm_device_size_op_t size;

	dm_device_query_op_t query;

	dm_device_prelink_op_t prelink;
	dm_device_unlink_cleanup_op_t unlink_cleanup;

	dm_device_destroy_op_t destroy;
} dm_device_ops_t;

PBOS_API km_result_t dm_register_device_class(const kf_uuid_t *uuid, dm_device_class_t **device_class_out);
PBOS_API dm_device_class_t *dm_query_device_class(const kf_uuid_t *uuid);
PBOS_API void dm_unregister_device_class(dm_device_class_t *device_class);

PBOS_API km_result_t dm_create_device(dm_bus_t *bus, dm_device_class_t *device_class, const dm_device_ops_t *ops, dm_device_t **device_out);

PBOS_API void dm_ref_device(dm_device_t *device);
PBOS_API void dm_unref_device(dm_device_t *device);

PBOS_API km_result_t dm_link_device(dm_device_t *parent, dm_device_t *device);
PBOS_API void dm_unlink_device(dm_device_t *device);

PBOS_API void dm_set_device_exdata(dm_device_t *device, void *exdata);
PBOS_API void *dm_get_device_exdata(dm_device_t *device);

///
/// @brief Get fnode object of the devio root directory, the fnode object returned will not be referenced.
///
/// @return Pointer to the devio root directory fnode object.
///
PBOS_API fs_fnode_t *dm_get_devio_root_dir();
PBOS_API km_result_t dm_create_devio_file(dm_device_t *device, fs_fnode_t *parent, const char *filename, size_t filename_len, fs_fnode_t **fnode_out);
PBOS_API km_result_t dm_create_devio_dir(fs_fnode_t *parent, const char *filename, size_t filename_len, fs_fnode_t **fnode_out);
PBOS_API km_result_t dm_remove_devio_fnode(fs_fnode_t *fnode);

PBOS_EXTERN_C_END

#endif
