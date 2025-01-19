#ifndef _PBOS_FS_FS_H_
#define _PBOS_FS_FS_H_

#include <pbos/km/objmgr.h>
#include <pbos/km/result.h>

typedef struct _fs_file_t fs_file_t;
typedef struct _fs_fcontext_t fs_fcontext_t;

typedef struct _fs_fsops_t {
	/// @brief Open a file.
	km_result_t (*open)(om_handle_t file, fs_fcontext_t **fcontext_out);
	/// @brief Close a FCB.
	void (*close)(fs_fcontext_t *fcontext);
	/// @brief Read data from a file.
	km_result_t (*read)(fs_fcontext_t *fcontext, char *dest, size_t size, size_t off, size_t *bytes_read_out);
	/// @brief Write data into a file.
	km_result_t (*write)(fs_fcontext_t *fcontext, const char *src, size_t size, size_t off, size_t *bytes_written_out);
	/// @brief Get size of a file.
	km_result_t (*size)(fs_fcontext_t *fcontext, size_t *size_out);

	km_result_t (*premount)(om_handle_t parent, om_handle_t file_handle);
	km_result_t (*postmount)(om_handle_t parent, om_handle_t file_handle);
	void (*mountfail)(om_handle_t parent, om_handle_t file_handle);
	km_result_t (*unmount)(om_handle_t file_handle);

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

#endif
