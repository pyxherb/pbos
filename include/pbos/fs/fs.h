#ifndef _PBOS_FS_FS_H_
#define _PBOS_FS_FS_H_

#include <pbos/km/objmgr.h>
#include <pbos/km/result.h>

PBOS_EXTERN_C_BEGIN

typedef struct _fs_file_t fs_file_t;
typedef struct _fs_fcb_t fs_fcb_t;

typedef struct _fs_fsops_t {
	/// @brief Access subnode.
	km_result_t (*subnode)(fs_file_t *parent, const char *name, size_t name_len, fs_file_t **file_out);
	/// @brief Offload a file node from the memory.
	void (*offload)(fs_file_t *file);
	/// @brief Create a new file.
	km_result_t (*create_file)(fs_file_t *parent, const char *name, size_t name_len, fs_file_t **file_out);
	/// @brief Create a new directory.
	km_result_t (*create_dir)(fs_file_t *parent, const char *name, size_t name_len, fs_file_t **file_out);
	/// @brief Open a file.
	km_result_t (*open)(fs_file_t *file, fs_fcb_t **fcb_out);
	/// @brief Close a FCB.
	km_result_t (*close)(fs_fcb_t *fcb);
	/// @brief Read data from a file.
	km_result_t (*read)(fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out);
	/// @brief Write data into a file.
	km_result_t (*write)(fs_fcb_t *fcb, const char *src, size_t size, size_t off, size_t *bytes_written_out);
	/// @brief Get size of a file.
	km_result_t (*size)(fs_fcb_t *fcb, size_t *size_out);

	km_result_t (*mount)(fs_file_t *parent, fs_file_t *file);
	km_result_t (*premount)(fs_file_t *parent, fs_file_t *file);
	km_result_t (*postmount)(fs_file_t *parent, fs_file_t *file);
	void (*mountfail)(fs_file_t *parent, fs_file_t *file);
	km_result_t (*unmount)(fs_file_t *file);

	/// @brief Destructor of the file system.
	km_result_t (*destructor)();
} fs_fsops_t;

typedef struct _fs_filesys_t {
	kf_rbtree_node_t tree_header;

	char name[32];
	uuid_t uuid;
	fs_fsops_t ops;
} fs_filesys_t;

fs_filesys_t *fs_register_filesys(
	const char *name,
	uuid_t *uuid,
	fs_fsops_t *ops);

PBOS_EXTERN_C_END

#endif
