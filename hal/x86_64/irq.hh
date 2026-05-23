#ifndef _HAL_X86_64_IRQ_HH_
#define _HAL_X86_64_IRQ_HH_

#include <arch/x86_64/irq.h>
#include <pbos/hal/irq.h>
#include "mm.hh"

PBOS_EXTERN_C_BEGIN

typedef struct _hal_irq_context_t {
	uint32_t placeholder;
} hal_irq_context_t;

PBOS_NORETURN void isr_diverr();		// 0x00 Divide-by-zero Error
PBOS_NORETURN void isr_overflow();		// 0x04 Overflow
PBOS_NORETURN void isr_boundrange();	// 0x05 Bound Range Exceeded
PBOS_NORETURN void isr_invl_opcode();	// 0x06 Invalid Opcode
PBOS_NORETURN void isr_nofpu();			// 0x07 Device (FPU) Not Available
PBOS_NORETURN void isr_double_fault();	// 0x08 Double Fault
PBOS_NORETURN void isr_dev_invltss();	// 0x0a Invalid TSS
PBOS_NORETURN void isr_noseg();			// 0x0b Segment Not Present
PBOS_NORETURN void isr_stackerr();		// 0x0c Stack Segment Fault
PBOS_NORETURN void isr_prot();			// 0x0d General Protection Fault
PBOS_NORETURN void isr_pgfault();		// 0x0e Page Fault
PBOS_NORETURN void isr_fpuerr();		// 0x10 x87 Floating-Point Exception
PBOS_NORETURN void isr_alignchk();		// 0x11 Alignment Check
PBOS_NORETURN void isr_machchk();		// 0x12 Machine Check
PBOS_NORETURN void isr_simderr();		// 0x13 SIMD Floating-Point Exception
PBOS_NORETURN void isr_virterr();		// 0x14 Virtualization Exception
PBOS_NORETURN void isr_ctrlprot();		// 0x15 Control Protection Exception
PBOS_NORETURN void isr_hverr();			// 0x1c Hypervisor Injection Exception
PBOS_NORETURN void isr_vmmerr();		// 0x1d VMM Communication Exception
PBOS_NORETURN void isr_securityerr();	// 0x1e Security Exception

PBOS_NORETURN void hali_isr_timer();	 // IRQ0
void hali_isr_timer_impl(
	const uint64_t rdi,
	const uint64_t rsi,
	const uint64_t rdx,
	const uint64_t rcx,
	const uint64_t r8,
	const uint64_t r9,

	const uint64_t gs,
	const uint64_t fs,
	const uint64_t es,
	const uint64_t ds,
	const uint64_t r10,
	const uint64_t r11,
	const uint64_t r12,
	const uint64_t r13,
	const uint64_t r14,
	const uint64_t r15,
	const uint64_t rax,
	const uint64_t rbx,
	const uint64_t rbp,

	const uint64_t *const rsp_top);

PBOS_NORETURN void isr_stub();

void hali_set_sched_timer();

extern arch_gate_t hali_kidt[512];

void hali_calibrate_apic();

void hali_set_isr(hal_isr_t isr, size_t irq, uint8_t dpl, uint8_t gate_type);

PBOS_EXTERN_C_END

#endif
