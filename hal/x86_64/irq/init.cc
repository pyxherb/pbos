#include <arch/x86_64/apic.h>
#include <pbos/kd/logger.h>
#include <pbos/ps/proc.h>
#include <hal/x86_64/irq.hh>
#include <hal/x86_64/ps/syscall.hh>
#include <pbos/ki/mp/misc.hh>

PBOS_EXTERN_C_BEGIN

void *hali_lapic_pbase;
uint32_t *hali_lapic_vbase;

/*static void hali_remap_pic(uint8_t pic1_offset, uint8_t pic2_offset) {
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
}*/

void kh_irq_init() {
	for (ps_cpuid_t i = 0; i < mp_num_total_cpu; ++i) {
		ki_per_cpu_io_contexts[i]->interrupt_table = mm_kalloc(sizeof(arch_gate_t) * 256 * 2, sizeof(arch_gate_t) * 2);

		if (!ki_per_cpu_io_contexts[i]->interrupt_table)
			km_panic("Error allocating memory for IDT for CPU #%d", i);
	}

	arch_lidt(static_cast<arch_gate_t *>(ki_per_cpu_io_contexts[0]->interrupt_table), 512);

	// for (int i = 0; i < 16; ++i)
	// arch_mask_pic_irq(i);

	// hali_remap_pic(0x20, 0x28);
	// arch_disable_pic();

	// Check if the CPU has APIC.
	// if (!arch_has_apic())
	// km_panic("The kernel requires APIC support");
}

km_result_t kh_init_irq_for_cpu(ps_cpuid_t cpu) {
	arch_gate_t *idt = static_cast<arch_gate_t *>(ki_per_cpu_io_contexts[cpu]->interrupt_table);

	hali_set_isr(idt, hali_div_err_isr, 0x00, 0, GATE_TRAP386);
	hali_set_isr(idt, hali_overflow_isr, 0x04, 0, GATE_TRAP386);
	hali_set_isr(idt, hali_bound_range_exceeded_isr, 0x05, 0, GATE_TRAP386);
	hali_set_isr(idt, hali_invalid_opcode_isr, 0x06, 0, GATE_INT386);
	hali_set_isr(idt, hali_nofpu_isr, 0x07, 0, GATE_TRAP386);
	hali_set_isr(idt, hali_double_fault_isr, 0x08, 0, GATE_INT386);
	hali_set_isr(idt, hali_invalid_tss_isr, 0x0a, 0, GATE_INT386);
	hali_set_isr(idt, hali_seg_not_present_isr, 0x0b, 0, GATE_INT386);
	hali_set_isr(idt, hali_stack_seg_fault_isr, 0x0c, 0, GATE_INT386);
	hali_set_isr(idt, hali_general_protect_isr, 0x0d, 0, GATE_INT386);
	// hali_set_isr(isr_pgfault, 0x0e, 0, GATE_INT386);
	/*hali_set_isr(idt, hali_fpu_except_isr, 0x10, 0, GATE_INT386);
	hali_set_isr(idt, hali_align_check_isr, 0x11, 0, GATE_INT386);
	hali_set_isr(idt, hali_machine_check_isr, 0x12, 0, GATE_INT386);
	hali_set_isr(idt, hali_simd_except_isr, 0x13, 0, GATE_INT386);
	hali_set_isr(idt, hali_virt_except_isr, 0x14, 0, GATE_INT386);
	hali_set_isr(idt, hali_ctrl_protect_isr, 0x15, 0, GATE_INT386);
	hali_set_isr(idt, hali_hypervisor_inject_except_isr, 0x1c, 0, GATE_INT386);
	hali_set_isr(idt, hali_vmm_comm_except_isr, 0x1d, 0, GATE_INT386);
	hali_set_isr(idt, hali_security_err_isr, 0x1e, 0, GATE_INT386);*/

	hali_set_isr(idt, hali_timer_isr, 0x30, 0, GATE_INT386);

	hali_set_isr(idt, isr_syscall, 0xc0, 3, GATE_TRAP386);

	return KM_RESULT_OK;
}

PBOS_EXTERN_C_END
