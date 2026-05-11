#ifndef _PBOS_KF_ATOMIC_H_
#define _PBOS_KF_ATOMIC_H_

#include "basedefs.h"

PBOS_EXTERN_C_BEGIN

size_t kf_atomic_inc_size(volatile size_t *addr);
size_t kf_atomic_dec_size(volatile size_t *addr);
void kf_atomic_swap(volatile size_t *addr, size_t replacement_value);
size_t kf_atomic_cmp_swap(volatile size_t *addr, size_t expected_value, size_t replacement_value);

PBOS_EXTERN_C_END

#endif
