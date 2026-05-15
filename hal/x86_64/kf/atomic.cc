#include <pbos/kh/kf/atomic.h>
#include <arch/x86_64/atomic.h>

uint8_t kh_atomic_inc_u8(volatile uint8_t *addr) {
	return arch_atomic_xadd8(addr, 1);
}

uint8_t kh_atomic_dec_u8(volatile uint8_t *addr) {
	return arch_atomic_xsub8(addr, 1);
}

uint8_t kh_atomic_xchg_u8(volatile uint8_t *addr, uint8_t replacement_value) {
	return arch_atomic_xchg8(addr, replacement_value);
}

uint8_t kh_atomic_cmp_xchg_u8(volatile uint8_t *addr, uint8_t expected_value, uint8_t replacement_value) {
	return arch_atomic_cmpxchg8(addr, expected_value, replacement_value);
}

size_t kh_atomic_inc_size(volatile size_t *addr) {
	return arch_atomic_xadd64((volatile uint64_t *)addr, 1);
}

size_t kh_atomic_dec_size(volatile size_t *addr) {
	return arch_atomic_xsub64((volatile uint64_t *)addr, 1);
}

size_t kh_atomic_swap_size(volatile size_t *addr, size_t replacement_value) {
	return arch_atomic_xchg64((volatile uint64_t *)addr, replacement_value);
}

size_t kh_atomic_cmp_xchg_size(volatile size_t *addr, size_t expected_value, size_t replacement_value) {
	return arch_atomic_cmpxchg64((volatile uint64_t *)addr, expected_value, replacement_value);
}
