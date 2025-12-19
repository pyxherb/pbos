#ifndef _PBOS_FS_FILE_H_
#define _PBOS_FS_FILE_H_

#include <pbos/kf/hashmap.h>
#include <pbos/km/objmgr.h>
#include <stdint.h>

PBOS_EXTERN_C_BEGIN

#define FILE_CLASS_UUID UUID(f11e5157, f11e, c1a5, 1d0f, f11e0b15)

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

typedef struct _fs_filesys_t fs_filesys_t;

typedef struct _fs_file_t fs_file_t;

typedef struct _fs_finddata_t {
	kf_hashmap_node_t *node;
} fs_finddata_t;

///
/// @brief File Context Block (FCB).
///
typedef struct _fs_fcb_t {
	kf_rbtree_node_t node_header;  // Node header for process's opened FCB set.
	fs_file_t *file;
	char exdata[];
} fs_fcb_t;

typedef struct _fs_finddata_t fs_finddata_t;

PBOS_NODISCARD km_result_t fs_create_file(
	fs_file_t *parent,
	const char *filename,
	size_t filename_len,
	fs_file_t **file_out);
PBOS_NODISCARD km_result_t fs_create_dir(
	fs_file_t *parent,
	const char *filename,
	size_t filename_len,
	fs_file_t **file_out);

/// @brief Mount a file onto a directory.
/// @param parent Parent directory to mount.
/// @param file Handle of file to be mounted, the parent directory will take the ownership.
/// @return Execution result of the operation.
PBOS_NODISCARD km_result_t fs_mount_file(fs_file_t *parent, fs_file_t *file);
PBOS_NODISCARD km_result_t fs_unmount_file(fs_file_t *file);

PBOS_NODISCARD km_result_t fs_close(fs_fcb_t *fcb);

PBOS_NODISCARD km_result_t fs_open(fs_file_t *base_dir, const char *path, size_t path_len, fs_fcb_t **fcb_out);
PBOS_NODISCARD km_result_t fs_read(fs_fcb_t *fcb, void *dest, size_t size, size_t off, size_t *bytes_read_out);
PBOS_NODISCARD km_result_t fs_write(fs_fcb_t *fcb, const char *src, size_t size, size_t off, size_t *bytes_written_out);
PBOS_NODISCARD km_result_t fs_size(fs_fcb_t *fcb, size_t *size_out);

PBOS_NODISCARD km_result_t fs_child_of(fs_file_t *file, const char *filename, size_t filename_len, fs_file_t **file_out);

///
/// @brief Resolve a path to a file.
///
/// @param cur_dir Current directory for resolution.
/// @param path Path to be resolved.
/// @param path_len Length of the path.
/// @param file_out Where the file object is received, note that the file will be reference counted.
/// @return Result of the resolution.
///
PBOS_NODISCARD km_result_t fs_resolve_path(fs_file_t *cur_dir, const char *path, size_t path_len, fs_file_t **file_out);

om_object_t *fs_file_to_object(fs_file_t *file);

extern om_class_t *fs_file_class;

PBOS_EXTERN_C_END

#endif
