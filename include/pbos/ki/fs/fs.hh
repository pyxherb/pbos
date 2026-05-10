#ifndef _PBOS_KI_FS_FS_H_
#define _PBOS_KI_FS_FS_H_

#include <pbos/fs/fs.h>
#include <pbos/fs/file.hh>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/kfxx/uuid.hh>
#include <pbos/kfxx/string_view.hh>

PBOS_EXTERN_C_BEGIN

typedef struct _fs_filesys_t : public kfxx::rbtree_t<kf_uuid_t>::node_t {
	char *name;
	size_t name_len;
	fs_fsops_t ops;

	_fs_filesys_t();
	~_fs_filesys_t();
} fs_filesys_t;

extern fs_fnode_t *fs_abs_root_dir;

/// @brief Initialize the file system facilities.
void fs_init();

PBOS_EXTERN_C_END

#endif
