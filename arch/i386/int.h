#ifndef _ARCH_I386_INT_H_
#define _ARCH_I386_INT_H_

#include "gate.h"
#include "port.h"

#define arch_iret() __asm__ __volatile__("iret")

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

#define ARCH_PIC1_IO_BASE 0x20
#define ARCH_PIC2_IO_BASE 0xa0
#define ARCH_PIC1_IO_COMMAND ARCH_PIC1_IO_BASE
#define ARCH_PIC1_IO_DATA (ARCH_PIC1_IO_BASE + 1)
#define ARCH_PIC2_IO_COMMAND ARCH_PIC2_IO_BASE
#define ARCH_PIC2_IO_DATA (ARCH_PIC2_IO_BASE + 1)

#define ARCH_PIC_COMMAND_EOI 0x20

#define ARCH_ICW1_COMMAND_ICW4 0x01		  // Indicates that ICW1 presents.
#define ARCH_ICW1_COMMAND_SINGLE 0x02	  // Single (cascade) mode.
#define ARCH_ICW1_COMMAND_INTERVAL4 0x04  // Call address interval 4.
#define ARCH_ICW1_COMMAND_LEVEL 0x08	  // Level triggered (edge) mode.
#define ARCH_ICW1_COMMAND_INIT 0x10		  // Initialize.

#define ARCH_ICW4_COMMAND_8086 0x01		   // 8086/88 (MCS-80/85) mode.
#define ARCH_ICW4_COMMAND_AUTO 0x02		   // Auto EOI
#define ARCH_ICW4_COMMAND_BUF_SLAVE 0x08   // Buffered mode/slave
#define ARCH_ICW4_COMMAND_BUF_MASTER 0x0c  // Buffered mode/master
#define ARCH_ICW4_COMMAND_SFNM 0x10		   // Special fully nested

static inline void arch_pic_eoi(uint8_t irq) {
	if (irq >= 8)
		arch_out8(ARCH_PIC2_IO_COMMAND, ARCH_PIC_COMMAND_EOI);

	arch_out8(ARCH_PIC1_IO_COMMAND, ARCH_PIC_COMMAND_EOI);
}

static inline void arch_disable_pic() {
	arch_out8(ARCH_PIC1_IO_DATA, 0xff);
	arch_out8(ARCH_PIC2_IO_DATA, 0xff);
}

static inline void arch_mask_irq(uint8_t irq) {
	if(irq < 8) {
		arch_out8(
			ARCH_PIC1_IO_DATA,
			arch_in8(ARCH_PIC1_IO_DATA) | (1 << irq));
	} else {
		arch_out8(
			ARCH_PIC2_IO_DATA,
			arch_in8(ARCH_PIC2_IO_DATA) | (1 << (irq - 8)));
	}
}

static inline void arch_unmask_irq(uint8_t irq) {
	if(irq < 8) {
		arch_out8(
			ARCH_PIC1_IO_DATA,
			arch_in8(ARCH_PIC1_IO_DATA) & ~(1 << irq));
	} else {
		arch_out8(
			ARCH_PIC2_IO_DATA,
			arch_in8(ARCH_PIC2_IO_DATA) & ~(1 << (irq - 8)));
	}
}

#define ARCH_PIC_COMMAND_READ_IRR 0x0a
#define ARCH_PIC_COMMAND_READ_ISR 0x0b

static inline uint16_t arch_read_pic_reg(uint8_t ocw3) {
	arch_out8(ARCH_PIC1_IO_COMMAND, ocw3);
	arch_out8(ARCH_PIC2_IO_COMMAND, ocw3);
	return (arch_in8(ARCH_PIC2_IO_COMMAND) << 8) | arch_in8(ARCH_PIC1_IO_COMMAND);
}

static inline uint16_t arch_read_pic_irr() {
	return arch_read_pic_reg(ARCH_PIC_COMMAND_READ_IRR);
}

static inline uint16_t arch_read_pic_isr() {
	return arch_read_pic_reg(ARCH_PIC_COMMAND_READ_ISR);
}

static inline void arch_ack_irq() {
	arch_out8(0x20, 0x20);
}

#endif
