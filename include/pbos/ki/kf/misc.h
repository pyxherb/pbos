#ifndef _PBOS_KI_KF_MISC_H_
#define _PBOS_KI_KF_MISC_H_

#include <pbos/kf/misc.h>

PBOS_NO_ASAN void *ki_raw_memcpy(void *dest, const void *src, size_t n);
PBOS_NO_ASAN void *ki_raw_memmove(void *dest, const void *src, size_t n);
PBOS_NO_ASAN void *ki_raw_memset(void *dest, int c, size_t n);

#endif
