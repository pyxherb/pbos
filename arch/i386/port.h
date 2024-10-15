#ifndef _ARCH_I386_PORT_H_
#define _ARCH_I386_PORT_H_

#include <pbos/common.h>

///
/// @brief Read 1 byte from a port.
///
/// @param port Port address.
/// @return Fetched value.
///
static inline uint8_t arch_in8(uint16_t port) {
	uint8_t data;
	__asm__ __volatile__("inb %1,%0"
						 : "=a"(data)
						 : "d"(port));
	return data;
}

///
/// @brief Read 2 bytes from a port.
///
/// @param port Port address.
/// @return Fetched value.
///
static inline uint16_t arch_in16(uint16_t port) {
	uint16_t data;
	__asm__ __volatile__("inw %1,%0"
						 : "=a"(data)
						 : "d"(port));
	return data;
}

///
/// @brief Read 4 bytes from a port.
///
/// @param port Port address.
/// @return Fetched value.
///
static inline uint32_t arch_in32(uint16_t port) {
	uint32_t data;
	__asm__ __volatile__("inl %1,%0"
						 : "=a"(data)
						 : "d"(port));
	return data;
}

///
/// @brief Write single byte of value into a port.
///
/// @param port Port address.
/// @param data Data to write.
///
static inline void arch_out8(uint16_t port, uint8_t data) {
	__asm__ __volatile__("outb %0,%1" ::"a"(data), "d"(port));
}

///
/// @brief Write 2 bytes of value into a port.
///
/// @param port Port address.
/// @param data Data to write.
///
static inline void arch_out16(uint16_t port, uint16_t data) {
	__asm__ __volatile__("outw %0,%1" ::"a"(data), "d"(port));
}

///
/// @brief Write 4 bytes of value into a port.
///
/// @param port Port address.
/// @param data Data to write.
///
static inline void arch_out32(uint16_t port, uint32_t data) {
	__asm__ __volatile__("outl %0,%1" ::"a"(data), "d"(port));
}

#endif
