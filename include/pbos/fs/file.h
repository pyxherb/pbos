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
	struct _fs_file_t *parent;

	fs_filesys_t *fs;
	fs_filetype_t filetype;

	size_t filename_len;
	char *filename;

	char exdata[];
} fs_file_t;

typedef struct _fs_finddata_t {
	kf_hashmap_node_t *node;
} fs_finddata_t;

typedef struct _fs_fcontext_t {
	kf_rbtree_node_t node_header;  // Node header for process's opened fcontext set.
	fs_file_t *file;
	char exdata[];
} fs_fcontext_t;

typedef struct _fs_finddata_t fs_finddata_t;

#define fs_file_exdata(file) ((file)->exdata)
#define fs_dir_exdata(file) (((fs_dir_exdata_t *)(file)->exdata)->exdata)

km_result_t fs_create_file(
	fs_file_t *parent,
	const char *filename,
	size_t filename_len,
	fs_file_t **file_out);
km_result_t fs_create_dir(
	fs_file_t *parent,
	const char *filename,
	size_t filename_len,
	fs_file_t **file_out);

/// @brief Mount a file onto a directory.
/// @param parent Parent directory to mount.
/// @param file Handle of file to be mounted, the parent directory will take the ownership.
/// @return Execution result of the operation.
km_result_t fs_mount_file(fs_file_t *parent, fs_file_t *file);
km_result_t fs_unmount_file(fs_file_t *file);

km_result_t fs_close(fs_fcontext_t *fcontext);

km_result_t fs_open(fs_file_t *base_dir, const char *path, size_t path_len, fs_fcontext_t **fcontext_out);
km_result_t fs_read(fs_fcontext_t *fcontext, void *dest, size_t size, size_t off, size_t *bytes_read_out);
km_result_t fs_write(fs_fcontext_t *fcontext, const char *src, size_t size, size_t off, size_t *bytes_written_out);
km_result_t fs_size(fs_fcontext_t *fcontext, size_t *size_out);

km_result_t fs_child_of(fs_file_t *file, const char *filename, size_t filename_len, fs_file_t **file_out);

km_result_t fs_resolve_path(fs_file_t *cur_dir, const char *path, size_t path_len, fs_file_t **file_out);

extern om_class_t *fs_file_class;

#endif
