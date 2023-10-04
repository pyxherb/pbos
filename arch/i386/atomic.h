#ifndef _ARCH_I386_ATOMIC_H_
#define _ARCH_I386_ATOMIC_H_

#include <oicos/attribs.h>
#include <oicos/common.h>

__always_inline static inline uint8_t arch_xchg8(volatile uint8_t *addr, uint8_t value) {
	uint8_t result;
	__asm__ __volatile__(
		"lock;xchgb %0,%1"
		: "+m"(*addr), "=a"(result)
		: "1"(value)
		: "cc");
	return result;
}

__always_inline static inline uint16_t arch_xchg16(volatile uint16_t *addr, uint16_t value) {
	uint16_t result;
	__asm__ __volatile__(
		"lock;xchgw %0,%1"
		: "+m"(*addr), "=a"(result)
		: "1"(value)
		: "cc");
	return result;
}

__always_inline static inline uint32_t arch_xchg32(volatile uint32_t *addr, uint32_t value) {
	uint32_t result;
	__asm__ __volatile__(
		"lock;xchgl %0,%1"
		: "+m"(*addr), "=a"(result)
		: "1"(value)
		: "cc");
	return result;
}

#endif
