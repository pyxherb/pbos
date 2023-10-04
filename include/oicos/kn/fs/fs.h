#ifndef _OICOS_KN_FS_FS_H_
#define _OICOS_KN_FS_FS_H_

#include <oicos/fs/fs.h>
#include <oicos/kf/hashmap.h>
#include <oicos/kf/rbtree.h>
#include <oicos/km/objmgr.h>

typedef struct _fs_context_t {
	om_handle_t root_dir, cur_dir;
} fs_context_t;

extern om_handle_t fs_abs_root_dir;

/// @brief Initialize the file system facilities.
void fs_init();

#endif
