#include <hal/i386/irq.h>
#include <hal/i386/syscall.h>
#include <pbos/km/logger.h>

static void hn_remap_pic(uint8_t pic1_offset, uint8_t pic2_offset) {
	uint8_t pic1_mask = arch_in8(ARCH_PIC1_IO_DATA), pic2_mask = arch_in8(ARCH_PIC2_IO_DATA);

	// Initialize both PICs.
	arch_out8(ARCH_PIC1_IO_COMMAND, ARCH_ICW1_COMMAND_INIT | ARCH_ICW1_COMMAND_ICW4);
	arch_io_wait();
	arch_out8(ARCH_PIC2_IO_COMMAND, ARCH_ICW1_COMMAND_INIT | ARCH_ICW1_COMMAND_ICW4);
	arch_io_wait();

	// Set master PIC vector offset.
	arch_out8(ARCH_PIC1_IO_DATA, pic1_offset);
	arch_io_wait();

	// Set slave PIC vector offset.
	arch_out8(ARCH_PIC2_IO_DATA, pic2_offset);
	arch_io_wait();

	arch_out8(ARCH_PIC1_IO_DATA, 4);
	arch_io_wait();

	arch_out8(ARCH_PIC2_IO_DATA, 2);
	arch_io_wait();

	// Tell the PICs to use 8086 mode instead of 8080 mode.
	arch_out8(ARCH_PIC1_IO_DATA, ARCH_ICW4_COMMAND_8086);
	arch_io_wait();
	arch_out8(ARCH_PIC2_IO_DATA, ARCH_ICW4_COMMAND_8086);
	arch_io_wait();

	// Restore saved masks.
	arch_out8(ARCH_PIC1_IO_DATA, pic1_mask);
	arch_out8(ARCH_PIC2_IO_DATA, pic2_mask);
}

void irq_init() {
	hn_setisr(isr_diverr, 0x00, 0, GATE_TRAP386);
	hn_setisr(isr_overflow, 0x04, 0, GATE_TRAP386);
	hn_setisr(isr_boundrange, 0x05, 0, GATE_TRAP386);
	hn_setisr(isr_invl_opcode, 0x06, 0, GATE_INT386);
	hn_setisr(isr_nofpu, 0x07, 0, GATE_TRAP386);
	hn_setisr(isr_double_fault, 0x08, 0, GATE_INT386);
	hn_setisr(isr_dev_invltss, 0x0a, 0, GATE_INT386);
	hn_setisr(isr_noseg, 0x0b, 0, GATE_INT386);
	hn_setisr(isr_stackerr, 0x0c, 0, GATE_INT386);
	hn_setisr(isr_prot, 0x0d, 0, GATE_INT386);
	// hn_setisr(isr_pgfault, 0x0e, 0, GATE_INT386);
	hn_setisr(isr_fpuerr, 0x10, 0, GATE_INT386);
	hn_setisr(isr_alignchk, 0x11, 0, GATE_INT386);
	hn_setisr(isr_machchk, 0x12, 0, GATE_INT386);
	hn_setisr(isr_simderr, 0x13, 0, GATE_INT386);
	hn_setisr(isr_virterr, 0x14, 0, GATE_INT386);
	hn_setisr(isr_ctrlprot, 0x15, 0, GATE_INT386);
	hn_setisr(isr_hverr, 0x1c, 0, GATE_INT386);
	hn_setisr(isr_vmmerr, 0x1d, 0, GATE_INT386);
	hn_setisr(isr_securityerr, 0x1e, 0, GATE_INT386);

	hn_setisr(isr_irq0, 0x20, 0, GATE_INT386);

	hn_setisr(isr_syscall, 0xc0, 3, GATE_TRAP386);

	arch_lidt(hn_kidt, 256);

	for (int i = 1; i < 16; ++i)
		arch_mask_irq(i);
	arch_unmask_irq(0);

	hn_remap_pic(0x20, 0x28);

	kdprintf("Initialized IRQ\n");
}
