#include <pbos/km/logger.h>
#include <hal/i386/irq.h>
#include <hal/i386/syscall.h>

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
	
	hn_setisr(isr_syscall, 1, 3, GATE_TRAP386);

	arch_lidt(hn_kidt, 2);
	
	kdprintf("Initialized IRQ\n");
}
