#ifndef _HAL_I386_IRQ_H_
#define _HAL_I386_IRQ_H_

#include <arch/i386/irq.h>
#include <pbos/hal/irq.h>
#include "mm.h"

PBOS_NORETURN void isr_diverr();		  // 0x00 Divide-by-zero Error
PBOS_NORETURN void isr_overflow();	  // 0x04 Overflow
PBOS_NORETURN void isr_boundrange();	  // 0x05 Bound Range Exceeded
PBOS_NORETURN void isr_invl_opcode();	  // 0x06 Invalid Opcode
PBOS_NORETURN void isr_nofpu();		  // 0x07 Device (FPU) Not Available
PBOS_NORETURN void isr_double_fault();  // 0x08 Double Fault
PBOS_NORETURN void isr_dev_invltss();	  // 0x0a Invalid TSS
PBOS_NORETURN void isr_noseg();		  // 0x0b Segment Not Present
PBOS_NORETURN void isr_stackerr();	  // 0x0c Stack Segment Fault
PBOS_NORETURN void isr_prot();		  // 0x0d General Protection Fault
PBOS_NORETURN void isr_pgfault();		  // 0x0e Page Fault
PBOS_NORETURN void isr_fpuerr();		  // 0x10 x87 Floating-Point Exception
PBOS_NORETURN void isr_alignchk();	  // 0x11 Alignment Check
PBOS_NORETURN void isr_machchk();		  // 0x12 Machine Check
PBOS_NORETURN void isr_simderr();		  // 0x13 SIMD Floating-Point Exception
PBOS_NORETURN void isr_virterr();		  // 0x14 Virtualization Exception
PBOS_NORETURN void isr_ctrlprot();	  // 0x15 Control Protection Exception
PBOS_NORETURN void isr_hverr();		  // 0x1c Hypervisor Injection Exception
PBOS_NORETURN void isr_vmmerr();		  // 0x1d VMM Communication Exception
PBOS_NORETURN void isr_securityerr();	  // 0x1e Security Exception

PBOS_NORETURN void isr_irq0();  // IRQ0
PBOS_NORETURN void isr_irq0_impl(
	const uint32_t eax,
	const uint32_t ebx,
	const uint32_t ecx,
	const uint32_t edx,
	const uint32_t esi,
	const uint32_t edi,
	const uint32_t ebp,
	const uint32_t eip,
	const uint32_t cs,
	const uint32_t eflags,
	const uint32_t esp);

PBOS_NORETURN void isr_syscall();

PBOS_NORETURN void isr_stub();

extern arch_gate_t hn_kidt[256];

void hn_setisr(hal_isr_t isr, size_t irq, uint8_t dpl, uint8_t gate_type);

#endif
