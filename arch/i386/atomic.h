#ifndef _ARCH_I386_ATOMIC_H_
#define _ARCH_I386_ATOMIC_H_

#include <pbos/attribs.h>
#include <pbos/common.h>

PB_FORCEINLINE static uint8_t arch_xchg8(volatile uint8_t *addr, uint8_t value) {
	uint8_t result;
	__asm__ __volatile__(
		"lock;xchgb %0,%1"
		: "+m"(*addr), "=a"(result)
		: "1"(value)
		: "cc");
	return result;
}

PB_FORCEINLINE static uint16_t arch_xchg16(volatile uint16_t *addr, uint16_t value) {
	uint16_t result;
	__asm__ __volatile__(
		"lock;xchgw %0,%1"
		: "+m"(*addr), "=a"(result)
		: "1"(value)
		: "cc");
	return result;
}

PB_FORCEINLINE static uint32_t arch_xchg32(volatile uint32_t *addr, uint32_t value) {
	uint32_t result;
	__asm__ __volatile__(
		"lock;xchgl %0,%1"
		: "+m"(*addr), "=a"(result)
		: "1"(value)
		: "cc");
	return result;
}

PB_FORCEINLINE static uint8_t arch_cmpxchg8(volatile uint8_t *addr, uint8_t expected_value, uint8_t replacement_value) {
	uint8_t result;
	__asm__ __volatile__(
		"movb %0, %%al"
		:
		: "r"(expected_value)
		: "%al");
	__asm__ __volatile__(
		"lock;"
		"cmpxchgb %0, %1"
		: "=a"(result)
		: "m"(*addr), "r"(replacement_value)
		: "memory");
	return result;
}

PB_FORCEINLINE static uint16_t arch_cmpxchg16(volatile uint16_t *addr, uint16_t expected_value, uint16_t replacement_value) {
	uint16_t result;
	__asm__ __volatile__(
		"movw %0, %%ax"
		:
		: "r"(expected_value)
		: "%ax");
	__asm__ __volatile__(
		"lock;"
		"cmpxchgw %0, %1"
		: "=a"(result)
		: "m"(*addr), "r"(replacement_value));
	return result;
}

PB_FORCEINLINE static uint32_t arch_cmpxchg32(volatile uint32_t *addr, uint32_t expected_value, uint32_t replacement_value) {
	uint32_t result;
	__asm__ __volatile__(
		"movl %0, %%eax"
		:
		: "r"(expected_value)
		: "%ax");
	__asm__ __volatile__(
		"lock;"
		"cmpxchgl %0, %1"
		: "=a"(result)
		: "m"(*addr), "r"(replacement_value));
	return result;
}

#endif
