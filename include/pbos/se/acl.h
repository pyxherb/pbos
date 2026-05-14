#ifndef _PBOS_SE_ACL_H_
#define _PBOS_SE_ACL_H_

#include <pbos/km/result.h>
#include <pbos/kf/uuid.h>

/// @brief ACD mode indicating that the access checker should use the mode from the parent.
#define SE_ACD_MODE_INHERITED 0
/// @brief ACD mode indicating the access is allowed.
#define SE_ACD_MODE_ALLOWED 1
/// @brief ACD mode indicating the access is prohibited
#define SE_ACD_MODE_PROHIBITED 2
/// @brief ACD mode indicating that a user filter will compute and return if the access is allowed.
#define SE_ACD_MODE_USER_FILTER 3

/// @brief The access control descriptor mode.
typedef uint8_t se_acd_mode_t;

#define SE_PERM_UUID_LOAD_KMOD KF_UUID(8f036ddb, a55c, 4d40, 88c5, 02e3f3949e62)

/// @brief The permission user filter callback type, the callback must return either @c SE_ACD_MODE_INHERITED or @c SE_ACD_MODE_ALLOWED or @c SE_ACD_MODE_PROHIBITED.
typedef se_acd_mode_t (*se_perm_user_filter)();

/// @brief The opaque access control descriptor (ACD) structure.
typedef struct _se_acd_t se_acd_t;

/// @brief The opaque access control list (ACL) structure.
typedef struct _se_acl_t se_acl_t;

#endif
