#ifndef _PBOS_SE_USER_H_
#define _PBOS_SE_USER_H_

#include <stdint.h>

#define SE_UID_ANY UINT32_MAX
#define SE_GID_ANY UINT32_MAX

#define SE_UID_ROOT 0

typedef uint32_t se_uid_t;
typedef uint32_t se_gid_t;

se_gid_t se_group_of_user(se_uid_t uid);

#endif
