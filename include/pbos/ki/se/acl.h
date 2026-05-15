#ifndef _PBOS_KI_SE_ACL_H_
#define _PBOS_KI_SE_ACL_H_

#include <pbos/se/acl.h>
#include <pbos/kfxx/map.hh>
#include <pbos/kfxx/uuid.hh>

/// @brief Definition of the opaque @c se_acd_t structure, the @c rb_value is the permission to be controlled.
typedef struct _se_acd_t : public kfxx::RBTree<kf_uuid_t>::Node {
	/// @brief Permission mode for this kind of access.
	se_acd_mode_t mode;
} se_acd_t;

/// @brief Definition of the opaque @c se_acl_t.
typedef struct _se_acl_t {
	/// @brief Tree that contains access control descriptors.
	kfxx::RBTree<kf_uuid_t> acd_tree;
} se_acl_t;

#endif
