#ifndef _PBOS_KF_ATOMIC_H_
#define _PBOS_KF_ATOMIC_H_

#include "basedefs.h"

PBOS_EXTERN_C_BEGIN

uint8_t kf_atomic_inc_u8(volatile uint8_t *addr);
uint8_t kf_atomic_dec_u8(volatile uint8_t *addr);
uint8_t kf_atomic_xchg_u8(volatile uint8_t *addr, uint8_t replacement_value);
uint8_t kf_atomic_cmp_xchg_u8(volatile uint8_t *addr, uint8_t expected_value, uint8_t replacement_value);

size_t kf_atomic_inc_size(volatile size_t *addr);
size_t kf_atomic_dec_size(volatile size_t *addr);
size_t kf_atomic_xchg_size(volatile size_t *addr, size_t replacement_value);
size_t kf_atomic_cmp_xchg_size(volatile size_t *addr, size_t expected_value, size_t replacement_value);

PBOS_EXTERN_C_END

#endif
