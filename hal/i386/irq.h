#ifndef _HAL_I386_IRQ_H_
#define _HAL_I386_IRQ_H_

#include "mm.h"

#include <arch/i386/int.h>
#include <pbos/hal/irq.h>

void __noreturn isr_diverr(); // 0x00 Divide-by-zero Error
void __noreturn isr_overflow(); // 0x04 Overflow
void __noreturn isr_boundrange(); // 0x05 Bound Range Exceeded
void __noreturn isr_invl_opcode(); // 0x06 Invalid Opcode
void __noreturn isr_nofpu(); // 0x07 Device (FPU) Not Available
void __noreturn isr_double_fault(); // 0x08 Double Fault
void __noreturn isr_dev_invltss(); // 0x0a Invalid TSS
void __noreturn isr_noseg(); // 0x0b Segment Not Present
void __noreturn isr_stackerr(); // 0x0c Stack Segment Fault
void __noreturn isr_prot(); // 0x0d General Protection Fault
void __noreturn isr_pgfault(); // 0x0e Page Fault
void __noreturn isr_fpuerr(); // 0x10 x87 Floating-Point Exception
void __noreturn isr_alignchk(); // 0x11 Alignment Check
void __noreturn isr_machchk(); // 0x12 Machine Check
void __noreturn isr_simderr(); // 0x13 SIMD Floating-Point Exception
void __noreturn isr_virterr(); // 0x14 Virtualization Exception
void __noreturn isr_ctrlprot(); // 0x15 Control Protection Exception
void __noreturn isr_hverr(); // 0x1c Hypervisor Injection Exception
void __noreturn isr_vmmerr(); // 0x1d VMM Communication Exception
void __noreturn isr_securityerr(); // 0x1e Security Exception

void __noreturn isr_syscall();

void __noreturn isr_stub();

extern arch_gate_t hn_kidt[256];

void hn_setisr(hal_isr_t isr, size_t irq, uint8_t dpl, uint8_t gate_type);

#endif
