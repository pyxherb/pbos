#ifndef _PBOS_KN_FS_FILE_HH_
#define _PBOS_KN_FS_FILE_HH_

#include <pbos/fs/file.h>
#include <pbos/kf/hashmap.h>
#include <pbos/kf/list.h>
#include <pbos/km/objmgr.h>
#include "fs.hh"

PBOS_EXTERN_C_BEGIN

///
/// @brief Allocate a new file node.
///
/// @param fs File system to be associated to the file node.
/// @param filename Name of the file node.
/// @param filename_len Length of name of the file node.
/// @param filetype Type of the file node.
/// @param exdata_size Size of extra data of the file node.
/// @param file_out Where the created file node is received.
/// @return Result code of allocation.
///
km_result_t kn_alloc_file(
	fs_filesys_t *fs,
	const char *filename,
	size_t filename_len,
	fs_filetype_t filetype,
	size_t exdata_size,
	fs_file_t** file_out);

void kn_file_destructor(om_object_t *obj);

PBOS_EXTERN_C_END

#endif
