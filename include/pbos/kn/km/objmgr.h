#ifndef _PBOS_KN_KM_OBJMGR_H_
#define _PBOS_KN_KM_OBJMGR_H_

#include <pbos/kf/rbtree.h>
#include <pbos/km/objmgr.h>

extern om_class_t *kn_class_list;
extern kf_rbtree_t kn_global_handles;

typedef struct _kn_handle_registry_t {
	kf_rbtree_node_t tree_header;

	om_handle_t handle;
	om_object_t *object;
} kn_handle_registry_t;

kn_handle_registry_t *kn_lookup_handle_registry(om_handle_t handle);

void om_init();

#endif
