#ifndef _PBOS_SE_USER_H_
#define _PBOS_SE_USER_H_

#include <stdint.h>

#define SE_UID_ANY UINT32_MAX
#define SE_GID_ANY UINT16_MAX

#define SE_UID_ROOT 0

typedef uint32_t se_uid_t;
typedef uint16_t se_gid_t;

typedef struct _se_gmember_iterator_t se_gmember_iterator_t;

se_gid_t se_groupof(se_uid_t uid);

#endif
