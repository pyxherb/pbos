#ifndef _PBOS_FS_FS_H_
#define _PBOS_FS_FS_H_

#include <pbos/io/dispatch.h>
#include <pbos/kf/uuid.h>
#include "file.h"

PBOS_EXTERN_C_BEGIN

typedef struct _fs_fnode_t fs_fnode_t;
typedef struct _fs_fcb_t fs_fcb_t;

typedef km_result_t (*fs_filesys_subnode_op_t)(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
typedef void (*fs_filesys_offload_op_t)(fs_fnode_t *file);

typedef km_result_t (*fs_filesys_create_file_op_t)(io_dispatch_context_t *dc, fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
typedef km_result_t (*fs_filesys_create_dir_op_t)(io_dispatch_context_t *dc, fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);

typedef km_result_t (*fs_filesys_open_op_t)(fs_fnode_t *file, fs_fcb_t **fcb_out);
typedef void (*fs_filesys_close_op_t)(fs_fcb_t *fcb);

typedef km_result_t (*fs_filesys_seek_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, long off, fs_seek_mode_t mode);
typedef km_result_t (*fs_filesys_read_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, char *dest, size_t size, size_t *bytes_read_out);
typedef km_result_t (*fs_filesys_write_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, const void *src, size_t size, size_t *bytes_written_out);

typedef km_result_t (*fs_filesys_pread_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out);
typedef km_result_t (*fs_filesys_pwrite_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out);

typedef km_result_t (*fs_filesys_ioctl_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, uint32_t ioctl_code, void *data_in, size_t size_in, void *data_out, size_t size_out, void *args);

typedef km_result_t (*fs_filesys_size_op_t)(io_dispatch_context_t *dc, fs_fcb_t *fcb, size_t *size_out);

typedef km_result_t (*fs_filesys_enum_first_child_file_op_t)(fs_fnode_t *dir, fs_fnode_t **first_file_out);
typedef km_result_t (*fs_filesys_enum_next_file_op_t)(fs_fnode_t *cur_file, fs_fnode_t **next_file_out);

typedef void (*fs_filesys_destroy_file_op_t)(fs_fnode_t *file);

typedef km_result_t (*fs_filesys_premount_op_t)(fs_fnode_t *parent, fs_fnode_t *file);
typedef void (*fs_filesys_mount_fail_op_t)(fs_fnode_t *parent, fs_fnode_t *file);
typedef km_result_t (*fs_filesys_unmount_cleanup_op_t)(fs_fnode_t *file);

/// @brief Destructor of the file system.
typedef km_result_t (*fs_filesys_filesys_destructor_op_t)();

typedef struct _fs_filesys_ops_t {
	/// @brief Access subnode.
	fs_filesys_subnode_op_t subnode;

	/// @brief Offload a file node from the memory.
	fs_filesys_offload_op_t offload;

	/// @brief Create a new file.
	fs_filesys_create_file_op_t create_file;
	/// @brief Create a new directory.
	fs_filesys_create_dir_op_t create_dir;

	/// @brief Open a file.
	fs_filesys_open_op_t open;
	/// @brief Close a FCB.
	fs_filesys_close_op_t close;

	fs_filesys_seek_op_t seek;
	fs_filesys_read_op_t read;
	fs_filesys_write_op_t write;

	/// @brief Read data from a file.
	fs_filesys_pread_op_t pread;
	/// @brief Write data into a file.
	fs_filesys_pwrite_op_t pwrite;

	/// @brief Perform an I/O control operation.
	fs_filesys_ioctl_op_t ioctl;

	/// @brief Get size of a file.
	fs_filesys_size_op_t size;

	fs_filesys_enum_first_child_file_op_t enum_first_child_file;
	fs_filesys_enum_next_file_op_t enum_next_file;

	fs_filesys_destroy_file_op_t destroy;

	fs_filesys_premount_op_t premount;
	fs_filesys_mount_fail_op_t mount_fail;
	fs_filesys_unmount_cleanup_op_t unmount_cleanup;

	fs_filesys_filesys_destructor_op_t destructor;
} fs_filesys_ops_t;

typedef struct _fs_filesys_t fs_filesys_t;

PBOS_API fs_filesys_t *fs_register_file_system(
	const char *name,
	size_t name_len,
	fs_filesys_ops_t *ops,
	fs_filesys_t **fs_out);

PBOS_EXTERN_C_END

#endif
