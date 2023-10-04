#ifndef _OICOS_KN_FS_FILE_H_
#define _OICOS_KN_FS_FILE_H_

#include <oicos/fs/file.h>
#include <oicos/kf/hashmap.h>
#include <oicos/kf/list.h>
#include <oicos/km/objmgr.h>

km_result_t kn_create_file(
	fs_filesys_t *fs,
	const char *filename,
	size_t filename_len,
	fs_filetype_t filetype,
	size_t exdata_size,
	om_handle_t* handle_out,
	fs_file_t** file_out);

// filesist-file-clas-idof-fileobjs
#define FILE_CLASS_UUID UUID(f11e5157, f11e, c1a5, 1d0f, f11e0b15)

void kn_file_destructor(om_object_t *obj);

#endif
