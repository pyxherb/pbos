#ifndef _PBOS_FS_FILE_H_
#define _PBOS_FS_FILE_H_

#include <pbos/kf/basedefs.h>
#include <pbos/km/result.h>
#include <stdint.h>

PBOS_EXTERN_C_BEGIN

enum {
	FS_FILETYPE_FILE = 0,  // Regular file
	FS_FILETYPE_DIR,	   // Directory Entry
	FS_FILETYPE_LINK,	   // Link
	FS_FILETYPE_BLKDEV,	   // Block device
	FS_FILETYPE_CHARDEV,   // Character device
	FS_FILETYPE_PIPE,	   // Pipe
	FS_FILETYPE_SOCKET	   // Socket
};
typedef uint8_t fs_filetype_t;

#define FS_ACCESS_READ 0x0001	   // Read the file
#define FS_ACCESS_WRITE 0x0002	   // Write the file
#define FS_ACCESS_EXEC 0x0004	   // Execute the file
#define FS_ACCESS_DELETE 0x0008	   // Delete the file
#define FS_ACCESS_LOCK 0x0010	   // Lock the file
#define FS_ACCESS_RDMOD 0x0020	   // Read access modifier
#define FS_ACCESS_CHMOD 0x0040	   // Write access modifier
#define FS_ACCESS_LIST 0x0080	   // List children
#define FS_ACCESS_READ_XA 0x0100   // Read extra attributes (XA)
#define FS_ACCESS_WRITE_XA 0x0200  // Write extra attributes (XA)
typedef uint16_t fs_faccess_t;

typedef struct _fs_file_system_t fs_file_system_t;

///
/// @brief The file node structure.
///
/// @note We didn't design a built-in extra data system, which means you should maintain your extra data relationship by your own.
///
typedef struct _fs_fnode_t fs_fnode_t;

typedef struct _fs_finddata_t fs_finddata_t;

typedef size_t fs_fhandle_t;

///
/// @brief File Context Block (FCB).
///
typedef struct _fs_fcb_t fs_fcb_t;

typedef struct _fs_finddata_t fs_finddata_t;

PBOS_NODISCARD PBOS_API km_result_t fs_create_file(
	fs_fnode_t *parent,
	const char *filename,
	size_t filename_len,
	fs_fnode_t **file_out);
PBOS_NODISCARD PBOS_API km_result_t fs_create_dir(
	fs_fnode_t *parent,
	const char *filename,
	size_t filename_len,
	fs_fnode_t **file_out);
PBOS_NODISCARD PBOS_API km_result_t fs_create_fcb(
	fs_fnode_t *file,
	fs_fcb_t **fcb_out);

void PBOS_API ki_set_fcb_exdata(fs_fcb_t *fcb, void *exdata);
PBOS_NODISCARD PBOS_API void *ki_get_fcb_exdata(fs_fcb_t *fcb);

PBOS_NODISCARD PBOS_API km_result_t fs_alloc_file_fnode(fs_file_system_t *file_system, fs_fnode_t **file_out);
PBOS_NODISCARD PBOS_API km_result_t fs_alloc_dir_fnode(fs_file_system_t *file_system, fs_fnode_t **file_out);

PBOS_NODISCARD PBOS_API void *fs_get_fnode_exdata(fs_fnode_t *file);
PBOS_API void fs_set_fnode_exdata(fs_fnode_t *file, void *exdata);

PBOS_NODISCARD PBOS_API const char *fs_name_of_fnode(fs_fnode_t *file, size_t *len_out);
PBOS_API void fs_unname_fnode(fs_fnode_t *file);
PBOS_NODISCARD PBOS_API km_result_t fs_rename_fnode(fs_fnode_t *file, const char *name, size_t name_len);

PBOS_API PBOS_API fs_fnode_t *fs_file_of_fcb(fs_fcb_t *fcb);

/// @brief Mount a file onto a directory.
/// @param parent Parent directory to mount, the directory must be empty.
/// @param file Handle of file to be mounted.
/// @return Execution result of the operation.
PBOS_NODISCARD PBOS_API km_result_t fs_mount_file(fs_fnode_t *parent, fs_fnode_t *file);
PBOS_NODISCARD PBOS_API km_result_t fs_unmount_file(fs_fnode_t *file);

PBOS_NODISCARD PBOS_API km_result_t fs_link_subnode(fs_fnode_t *parent, fs_fnode_t *file);

km_result_t fs_close(fs_fcb_t *fcb);

PBOS_NODISCARD PBOS_API km_result_t fs_open(fs_fnode_t *base_dir, const char *path, size_t path_len, fs_fcb_t **fcb_out);
PBOS_NODISCARD PBOS_API km_result_t fs_read(fs_fcb_t *fcb, void *dest, size_t size, size_t off, size_t *bytes_read_out);
PBOS_NODISCARD PBOS_API km_result_t fs_write(fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out);
PBOS_NODISCARD PBOS_API km_result_t fs_size(fs_fcb_t *fcb, size_t *size_out);

PBOS_NODISCARD PBOS_API km_result_t fs_child_of(fs_fnode_t *file, const char *filename, size_t filename_len, fs_fnode_t **file_out);

///
/// @brief Resolve a path to a file.
///
/// @param cur_dir Current directory for resolution.
/// @param path Path to be resolved.
/// @param path_len Length of the path.
/// @param file_out Where the file object is received, note that the file will be reference counted.
/// @return Result of the resolution.
///
PBOS_NODISCARD PBOS_API km_result_t fs_resolve_path(fs_fnode_t *cur_dir, const char *path, size_t path_len, fs_fnode_t **file_out);

PBOS_API PBOS_API void fs_ref_fnode(fs_fnode_t *fnode);
PBOS_API PBOS_API void fs_unref_fnode(fs_fnode_t *fnode);

PBOS_EXTERN_C_END

#endif
