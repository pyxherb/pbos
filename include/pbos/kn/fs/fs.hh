#ifndef _PBOS_KN_FS_FS_H_
#define _PBOS_KN_FS_FS_H_

#include <pbos/fs/fs.h>
#include <pbos/kf/hashmap.h>
#include <pbos/kf/rbtree.h>
#include <pbos/km/objmgr.h>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/kfxx/uuid.hh>
#include <pbos/kfxx/string_view.hh>

PBOS_EXTERN_C_BEGIN

typedef struct _fs_filesys_t : public kfxx::rbtree_t<kf_uuid_t>::node_t {
	kfxx::string_view name;
	fs_fsops_t ops;
} fs_filesys_t;

typedef struct _fs_context_t {
	fs_file_t *root_dir, *cur_dir;
} fs_context_t;

extern fs_file_t *fs_abs_root_dir;

/// @brief Initialize the file system facilities.
void fs_init();

PBOS_EXTERN_C_END

#endif
