#ifndef _PBOS_KI_FS_FS_H_
#define _PBOS_KI_FS_FS_H_

#include <pbos/fs/fs.h>
#include <pbos/fs/file.hh>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/kfxx/uuid.hh>
#include <pbos/kfxx/string_view.hh>

PBOS_EXTERN_C_BEGIN

typedef struct _fs_file_system_t {
	char *name;
	size_t name_len;
	fs_file_system_ops_t ops;

	_fs_file_system_t();
	~_fs_file_system_t();
} fs_file_system_t;

extern fs_fnode_t *fs_abs_root_dir;

/// @brief Initialize the file system facilities.
void ki_fs_init();

PBOS_EXTERN_C_END

#endif
