#ifndef _PBOS_KN_FS_FILE_HH_
#define _PBOS_KN_FS_FILE_HH_

#include <pbos/fs/file.h>
#include <pbos/kfxx/hashmap.hh>
#include <pbos/kf/list.h>
#include <pbos/km/objmgr.h>
#include "fs.hh"

PBOS_EXTERN_C_BEGIN

typedef struct _fs_fnode_t : public om_object_t {
	_fs_fnode_t *parent;

	fs_filesys_t *fs;
	fs_filetype_t filetype;

	size_t filename_len;
	char *filename;
} fs_fnode_t;

typedef struct _fs_file_t : public fs_fnode_t {
} fs_file_t;

typedef struct _fs_dir_t : public fs_fnode_t {
} fs_dir_t;

void kn_file_destructor(om_object_t *obj);

PBOS_EXTERN_C_END

#endif
