#include <pbos/kh/kf/atomic.h>
#include <arch/x86_64/atomic.h>

size_t kh_atomic_inc_size(volatile size_t *addr) {
	return arch_atomic_xadd64((volatile uint64_t *)addr, 1);
}

size_t kh_atomic_dec_size(volatile size_t *addr) {
	return arch_atomic_xsub64((volatile uint64_t *)addr, 1);
}

void kh_atomic_swap_size(volatile size_t *addr, size_t replacement_value) {
	arch_atomic_xchg64((volatile uint64_t *)addr, replacement_value);
}

size_t kh_atomic_cmp_swap_size(volatile size_t *addr, size_t expected_value, size_t replacement_value) {
	return arch_atomic_cmpxchg64((volatile uint64_t *)addr, expected_value, replacement_value);
}
