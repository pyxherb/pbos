#ifndef _PBOS_SE_PROC_H_
#define _PBOS_SE_PROC_H_

#include "user.h"

/// @brief Process Security Context (PSC)
typedef struct _se_psc_t {
	se_uid_t owner_uid;
} se_psc_t;

#endif
