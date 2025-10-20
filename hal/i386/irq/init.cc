#include <arch/i386/apic.h>
#include <hal/i386/irq.h>
#include <hal/i386/syscall.h>
#include <pbos/km/logger.h>
#include <pbos/km/proc.h>

PBOS_EXTERN_C_BEGIN

void *hn_lapic_pbase;
uint32_t *hn_lapic_vbase;

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

void hal_irq_init() {
	hn_setisr(isr_diverr, 0x00, 0, GATE_TRAP386);
	hn_setisr(isr_overflow, 0x04, 0, GATE_TRAP386);
	hn_setisr(isr_boundrange, 0x05, 0, GATE_TRAP386);
	hn_setisr(isr_invl_opcode, 0x06, 0, GATE_INT386);
	hn_setisr(isr_nofpu, 0x07, 0, GATE_TRAP386);
	// hn_setisr(isr_double_fault, 0x08, 0, GATE_INT386);
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

	hn_setisr(isr_timer, 0x30, 0, GATE_INT386);

	hn_setisr(isr_syscall, 0xc0, 3, GATE_INT386);

	arch_lidt(hn_kidt, 256);

	for (int i = 0; i < 16; ++i)
		arch_mask_irq(i);

	hn_remap_pic(0x20, 0x28);
	arch_disable_pic();

	// Check if the CPU has APIC.
	if (!arch_has_apic())
		km_panic("The kernel requires APIC support");

	// Relocate and remap APIC.
	if (!(hn_lapic_pbase = mm_pgalloc(MM_PMEM_AVAILABLE)))
		km_panic("Unable to allocate physical LAPIC page");

	if (!(hn_lapic_vbase = (uint32_t *)mm_kvmalloc(mm_kernel_context, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, 0)))
		km_panic("Unable to allocate virtual LAPIC page");

	// arch_set_apic_base(hn_lapic_pbase, ARCH_APIC_BASE_MSR_BSP | ARCH_APIC_BASE_MSR_ENABLE);

	if (KM_FAILED(mm_iommap(mm_kernel_context, hn_lapic_vbase, (void*)ARCH_DEFAULT_APIC_PBASE, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE | PAGE_NOCACHE, MMAP_IOREMAP))) {
		km_panic("Unable to mapping LAPIC page for the main EU");
	}

	// Allocate IRQ contexts.
	if (!(hal_irq_contexts = (hal_irq_context_t **)mm_kmalloc(ps_eu_num * sizeof(hal_irq_context_t *), alignof(hal_irq_context_t *)))) {
		km_panic("Unable to allocate interrupt context for all CPUs");
	}

	for (uint32_t i = 0; i < ps_eu_num; ++i) {
		if (!(hal_irq_contexts[i] = (hal_irq_context_t *)mm_kmalloc(ps_eu_num * sizeof(hal_irq_context_t), alignof(hal_irq_context_t)))) {
			km_panic("Unable to allocate interrupt context for CPU %u", i);
		}
	}

	kdprintf("Initialized IRQ\n");
}

PBOS_EXTERN_C_END
