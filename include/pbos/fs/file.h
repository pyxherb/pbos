#ifndef _PBOS_FS_FILE_H_
#define _PBOS_FS_FILE_H_

#include <pbos/kf/hashmap.h>
#include <pbos/km/objmgr.h>
#include <stdint.h>

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

typedef struct _fs_file_t {
	om_object_t object_header;
	om_handle_t parent;

	fs_filesys_t *fs;
	fs_filetype_t filetype;

	size_t filename_len;
	char *filename;

	char exdata[];
} fs_file_t;

/// @brief Extra data for directory files.
typedef struct _fs_dir_exdata_t {
	kf_hashmap_t children;
	char exdata[];
} fs_dir_exdata_t;

typedef struct _fs_finddata_t {
	kf_hashmap_node_t *node;
} fs_finddata_t;

typedef struct _fs_finddata_t fs_finddata_t;

#define fs_file_exdata(file) ((file)->exdata)
#define fs_dir_exdata(file) (((fs_dir_exdata_t *)(file)->exdata)->exdata)

km_result_t fs_deref_file_handle(
	om_handle_t file_handle,
	fs_file_t **file_out);
km_result_t fs_create_file(
	fs_filesys_t *fs,
	const char *filename,
	size_t filename_len,
	size_t exdata_size,
	om_handle_t *handle_out);
km_result_t fs_create_dir(
	fs_filesys_t *fs,
	const char *filename,
	size_t filename_len,
	size_t exdata_size,
	om_handle_t *handle_out);

/// @brief Mount a file onto a directory.
/// @param parent Parent directory to mount.
/// @param file_handle Handle of file to be mounted, the parent directory will take the ownership.
/// @return Execution result of the operation.
km_result_t fs_mount_file(om_handle_t parent, om_handle_t file_handle);
km_result_t fs_unmount_file(om_handle_t file_handle);

km_result_t fs_close_file(om_handle_t file_handle);

km_result_t fs_open(const char *path, size_t path_len, om_handle_t *handle_out);
km_result_t fs_read(om_handle_t file_handle, void *dest, size_t size, size_t off, size_t *bytes_read_out);
km_result_t fs_write(om_handle_t file_handle, const char *src, size_t size, size_t off, size_t *bytes_written_out);
km_result_t fs_size(om_handle_t file_handle, size_t *size_out);

km_result_t fs_child_of(om_handle_t file_handle, const char *filename, size_t filename_len, om_handle_t *handle_out);

km_result_t fs_resolve_path(om_handle_t file_handle, const char *path, size_t path_len, om_handle_t *handle_out);

km_result_t fs_find_file(om_handle_t file_handle, fs_finddata_t *finddata, om_handle_t *file_handle_out);
void fs_find_next_file(fs_finddata_t *finddata, om_handle_t *file_handle_out);

extern om_class_t *fs_file_class;

#endif
