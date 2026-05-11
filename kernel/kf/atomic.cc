#include <pbos/kh/kf/atomic.h>

PBOS_EXTERN_C_BEGIN

size_t kf_atomic_inc_size(volatile size_t *addr) {
	return kh_atomic_inc_size(addr);
}

size_t kf_atomic_dec_size(volatile size_t *addr) {
	return kh_atomic_dec_size(addr);
}

void kf_atomic_swap_size(volatile size_t *addr, size_t replacement_value) {
	kh_atomic_swap_size(addr, replacement_value);
}

size_t kf_atomic_cmp_swap_size(volatile size_t *addr, size_t expected_value, size_t replacement_value) {
	return kh_atomic_cmp_swap_size(addr, expected_value, replacement_value);
}

PBOS_EXTERN_C_END
