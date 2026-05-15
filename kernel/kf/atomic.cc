#include <pbos/kh/kf/atomic.h>

PBOS_EXTERN_C_BEGIN

uint8_t kf_atomic_inc_u8(volatile uint8_t *addr) {
	return kh_atomic_inc_u8(addr);
}

uint8_t kf_atomic_dec_u8(volatile uint8_t *addr) {
	return kh_atomic_dec_u8(addr);
}

uint8_t kf_atomic_xchg_u8(volatile uint8_t *addr, uint8_t replacement_value) {
	return kh_atomic_xchg_u8(addr, replacement_value);
}

uint8_t kf_atomic_cmp_xchg_u8(volatile uint8_t *addr, uint8_t expected_value, uint8_t replacement_value) {
	return kh_atomic_cmp_xchg_u8(addr, expected_value, replacement_value);
}

size_t kf_atomic_inc_size(volatile size_t *addr) {
	return kh_atomic_inc_size(addr);
}

size_t kf_atomic_dec_size(volatile size_t *addr) {
	return kh_atomic_dec_size(addr);
}

size_t kf_atomic_xchg_size(volatile size_t *addr, size_t replacement_value) {
	return kh_atomic_swap_size(addr, replacement_value);
}

size_t kf_atomic_cmp_xchg_size(volatile size_t *addr, size_t expected_value, size_t replacement_value) {
	return kh_atomic_cmp_xchg_size(addr, expected_value, replacement_value);
}

PBOS_EXTERN_C_END
