#ifndef _ARCH_X86_64_ATOMIC_H_
#define _ARCH_X86_64_ATOMIC_H_

#include <pbos/attribs.h>
#include <pbos/common.h>

PBOS_FORCEINLINE static void arch_atomic_inc8(volatile uint8_t *addr) {
	__asm__ __volatile__(
		"lock;incb %0"
		: "+m"(*addr)::"cc");
}

PBOS_FORCEINLINE static void arch_atomic_inc16(volatile uint16_t *addr) {
	__asm__ __volatile__(
		"lock;incw %0"
		: "+m"(*addr)::"cc");
}

PBOS_FORCEINLINE static void arch_atomic_inc32(volatile uint32_t *addr) {
	__asm__ __volatile__(
		"lock;incl %0"
		: "+m"(*addr)::"cc");
}

PBOS_FORCEINLINE static void arch_atomic_inc64(volatile uint64_t *addr) {
	__asm__ __volatile__(
		"lock;incq %0"
		: "+m"(*addr)::"cc");
}

PBOS_FORCEINLINE static void arch_atomic_dec8(volatile uint8_t *addr) {
	__asm__ __volatile__(
		"lock;decb %0"
		: "+m"(*addr)::"cc");
}

PBOS_FORCEINLINE static void arch_atomic_dec16(volatile uint16_t *addr) {
	__asm__ __volatile__(
		"lock;decw %0"
		: "+m"(*addr)::"cc");
}

PBOS_FORCEINLINE static void arch_atomic_dec32(volatile uint32_t *addr) {
	__asm__ __volatile__(
		"lock;decl %0"
		: "+m"(*addr)::"cc");
}

PBOS_FORCEINLINE static void arch_atomic_dec64(volatile uint64_t *addr) {
	__asm__ __volatile__(
		"lock;decq %0"
		: "+m"(*addr)::"cc");
}

PBOS_FORCEINLINE static uint8_t arch_atomic_xadd8(volatile uint8_t *addr, uint8_t value) {
	__asm__ __volatile__(
		"lock;xaddb %1, %0"
		: "+m"(*addr), "+r"(value) :
		: "cc");
	return value + 1;
}

PBOS_FORCEINLINE static uint16_t arch_atomic_xadd16(volatile uint16_t *addr, uint16_t value) {
	__asm__ __volatile__(
		"lock;xaddw %1, %0"
		: "+m"(*addr), "+r"(value) :
		: "cc");
	return value + 1;
}

PBOS_FORCEINLINE static uint32_t arch_atomic_xadd32(volatile uint32_t *addr, uint32_t value) {
	__asm__ __volatile__(
		"lock;xaddl %1, %0"
		: "+m"(*addr), "+r"(value) :
		: "cc");
	return value + 1;
}

PBOS_FORCEINLINE static uint64_t arch_atomic_xadd64(volatile uint64_t *addr, uint64_t value) {
	__asm__ __volatile__(
		"lock;xaddq %1, %0"
		: "+m"(*addr), "+r"(value) :
		: "cc");
	return value + 1;
}

PBOS_FORCEINLINE static uint8_t arch_atomic_xsub8(volatile uint8_t *addr, uint8_t value) {
	value = ~value + 1;
	__asm__ __volatile__(
		"lock;xaddb %1, %0"
		: "+m"(*addr), "+r"(value) :
		: "cc");
	return value - 1;
}

PBOS_FORCEINLINE static uint16_t arch_atomic_xsub16(volatile uint16_t *addr, uint16_t value) {
	value = ~value + 1;
	__asm__ __volatile__(
		"lock;xaddw %1, %0"
		: "+m"(*addr), "+r"(value) :
		: "cc");
	return value - 1;
}

PBOS_FORCEINLINE static uint32_t arch_atomic_xsub32(volatile uint32_t *addr, uint32_t value) {
	value = ~value + 1;
	__asm__ __volatile__(
		"lock;xaddl %1, %0"
		: "+m"(*addr), "+r"(value) :
		: "cc");
	return value - 1;
}

PBOS_FORCEINLINE static uint64_t arch_atomic_xsub64(volatile uint64_t *addr, uint64_t value) {
	value = ~value + 1;
	__asm__ __volatile__(
		"lock;xaddq %1, %0"
		: "+m"(*addr), "+r"(value) :
		: "cc");
	return value - 1;
}

PBOS_FORCEINLINE static uint8_t arch_atomic_xchg8(volatile uint8_t *addr, uint8_t value) {
	uint8_t result;
	__asm__ __volatile__(
		"lock;xchgb %0, %1"
		: "+m"(*addr), "=a"(result)
		: "1"(value)
		: "cc");
	return result;
}

PBOS_FORCEINLINE static uint16_t arch_atomic_xchg16(volatile uint16_t *addr, uint16_t value) {
	uint16_t result;
	__asm__ __volatile__(
		"lock;xchgw %0, %1"
		: "+m"(*addr), "=a"(result)
		: "1"(value)
		: "cc");
	return result;
}

PBOS_FORCEINLINE static uint32_t arch_atomic_xchg32(volatile uint32_t *addr, uint32_t value) {
	uint32_t result;
	__asm__ __volatile__(
		"lock;xchgl %0, %1"
		: "+m"(*addr), "=a"(result)
		: "1"(value)
		: "cc");
	return result;
}

PBOS_FORCEINLINE static uint64_t arch_atomic_xchg64(volatile uint64_t *addr, uint64_t value) {
	uint64_t result;
	__asm__ __volatile__(
		"lock;xchgq %0, %1"
		: "+m"(*addr), "=a"(result)
		: "1"(value)
		: "cc");
	return result;
}

PBOS_FORCEINLINE static uint8_t arch_atomic_cmpxchg8(volatile uint8_t *addr,
	uint8_t expected_value,
	uint8_t replacement_value) {
	uint8_t result;
	__asm__ __volatile__(
		"lock; cmpxchgb %[repl], %[dest]"
		: "=a"(result),
		[dest] "+m"(*addr)
		: "a"(expected_value),
		[repl] "r"(replacement_value)
		: "memory", "cc");
	return result;
}

PBOS_FORCEINLINE static uint16_t arch_atomic_cmpxchg16(volatile uint16_t *addr,
	uint16_t expected_value,
	uint16_t replacement_value) {
	uint16_t result;
	__asm__ __volatile__(
		"lock; cmpxchgW %[repl], %[dest]"
		: "=a"(result),
		[dest] "+m"(*addr)
		: "a"(expected_value),
		[repl] "r"(replacement_value)
		: "memory", "cc");
	return result;
}

PBOS_FORCEINLINE static uint32_t arch_atomic_cmpxchg32(volatile uint32_t *addr,
	uint32_t expected_value,
	uint32_t replacement_value) {
	uint32_t result;
	__asm__ __volatile__(
		"lock; cmpxchgL %[repl], %[dest]"
		: "=a"(result),
		[dest] "+m"(*addr)
		: "a"(expected_value),
		[repl] "r"(replacement_value)
		: "memory", "cc");
	return result;
}

PBOS_FORCEINLINE static uint64_t arch_atomic_cmpxchg64(volatile uint64_t *addr,
	uint64_t expected_value,
	uint64_t replacement_value) {
	uint64_t result;
	__asm__ __volatile__(
		"lock; cmpxchgq %[repl], %[dest]"
		: "=a"(result),
		[dest] "+m"(*addr)
		: "a"(expected_value),
		[repl] "r"(replacement_value)
		: "memory", "cc");
	return result;
}

#endif
