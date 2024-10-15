#ifndef _PBOS_KN_SE_USER_H_
#define _PBOS_KN_SE_USER_H_

#include <pbos/se/user.h>
#include <kf/rbtree.h>

typedef struct _kn_user_t {
	kf_rbtree_node_t tree_header;
	se_uid_t uid;
	se_gid_t group;
} kn_user_t;

typedef struct _kn_gmember_t {
	kf_rbtree_node_t tree_header;
	se_uid_t uid;
} kn_gmember_t;

typedef struct _kn_group_t {
	kf_rbtree_node_t tree_header;
	kf_rbtree_t members;
} kn_group_t;

kn_user_t* kn_lookup_user(se_uid_t uid);
kn_group_t* kn_lookup_group(se_gid_t gid);

#endif
