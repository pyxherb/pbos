#ifndef _ARCH_I386_INT_H_
#define _ARCH_I386_INT_H_

#include "gate.h"
#include "port.h"

#define arch_iret() __asm__ __volatile__("iret");

static inline void arch_cli() {
	__asm__ __volatile__("cli");
}

static inline void arch_sti() {
	__asm__ __volatile__("sti");
}

static inline void arch_mask_nmi() {
	arch_out8(0x70, arch_in8(0x70) | 0x80);
}

static inline void arch_unmask_nmi() {
	arch_out8(0x70, arch_in8(0x70) & 0x7f);
}

void arch_lidt(void *idt, uint16_t size);

#endif
