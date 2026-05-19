#ifndef _PBOS_FS_FS_H_
#define _PBOS_FS_FS_H_

#include <pbos/kf/uuid.h>
#include <pbos/km/result.h>

PBOS_EXTERN_C_BEGIN

typedef struct _fs_fnode_t fs_fnode_t;
typedef struct _fs_fcb_t fs_fcb_t;

typedef struct _fs_file_system_ops_t {
	/// @brief Access subnode.
	km_result_t (*subnode)(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
	/// @brief Offload a file node from the memory.
	void (*offload)(fs_fnode_t *file);
	/// @brief Create a new file.
	km_result_t (*create_file)(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
	/// @brief Create a new directory.
	km_result_t (*create_dir)(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out);
	/// @brief Open a file.
	km_result_t (*open)(fs_fnode_t *file, fs_fcb_t **fcb_out);
	/// @brief Close a FCB.
	void (*close)(fs_fcb_t *fcb);
	/// @brief Read data from a file.
	km_result_t (*read)(fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out);
	/// @brief Write data into a file.
	km_result_t (*write)(fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out);
	/// @brief Perform an I/O control operation.
	km_result_t (*ioctl)(fs_fcb_t *fcb, uint32_t ioctl_code, void *data_in, size_t size_in, void *data_out, size_t size_out, void *args);
	/// @brief Get size of a file.
	km_result_t (*size)(fs_fcb_t *fcb, size_t *size_out);

	km_result_t (*enum_first_child_file)(fs_fnode_t *dir, fs_fnode_t **first_file_out);
	km_result_t (*enum_next_file)(fs_fnode_t *cur_file, fs_fnode_t **next_file_out);

	void (*destroy)(fs_fnode_t *file);

	km_result_t (*premount)(fs_fnode_t *parent, fs_fnode_t *file);
	void (*mount_fail)(fs_fnode_t *parent, fs_fnode_t *file);
	km_result_t (*unmount_cleanup)(fs_fnode_t *file);

	/// @brief Destructor of the file system.
	km_result_t (*destructor)();
} fs_file_system_ops_t;

typedef struct _fs_file_system_t fs_file_system_t;

PBOS_API fs_file_system_t *fs_register_file_system(
	const char *name,
	size_t name_len,
	fs_file_system_ops_t *ops,
	fs_file_system_t **fs_out);

PBOS_EXTERN_C_END

#endif
