#ifndef _PBOS_KN_FS_FS_H_
#define _PBOS_KN_FS_FS_H_

#include <pbos/fs/fs.h>
#include <pbos/kf/hashmap.h>
#include <pbos/kf/rbtree.h>
#include <pbos/km/objmgr.h>

typedef struct _fs_context_t {
	om_handle_t root_dir, cur_dir;
} fs_context_t;

extern om_handle_t fs_abs_root_dir;

/// @brief Initialize the file system facilities.
void fs_init();

#endif
