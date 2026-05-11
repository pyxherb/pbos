#ifndef _PBOS_KH_KF_ATOMIC_H_
#define _PBOS_KH_KF_ATOMIC_H_

#include <pbos/kf/atomic.h>

PBOS_EXTERN_C_BEGIN

size_t kh_atomic_inc_size(volatile size_t *addr);
size_t  kh_atomic_dec_size(volatile size_t *addr);
void kh_atomic_swap_size(volatile size_t *addr, size_t replacement_value);
size_t kh_atomic_cmp_swap_size(volatile size_t *addr, size_t expected_value, size_t replacement_value);

PBOS_EXTERN_C_END

#endif
