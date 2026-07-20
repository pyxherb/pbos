#include <arch/x86_64/apic.h>
#include <pbos/kd/logger.h>
#include <hal/x86_64/irq.hh>
#include <hal/x86_64/proc.hh>

PBOS_EXTERN_C_BEGIN

size_t kh_get_irq_max() {
	return 255;
}

void hali_set_isr(arch_gate_t *idt, hali_isr_t isr, size_t irq, uint8_t dpl, uint8_t gate_type) {
	kd_assert(irq <= kh_get_irq_max());
	kd_assert(dpl <= 3);
	idt[(irq * 2)] = GATEDESC_LOW(isr, SELECTOR_KCODE, GATEDESC_ATTRIBS(1, dpl, 0, gate_type));
	idt[(irq * 2) + 1] = GATEDESC_HIGH(isr);
}

bool kh_is_per_cpu_irq_supported() {
	return true;
}

bool kh_is_irq_disabled() {
	return !(arch_rflags() & (1 << 9));
}

void kh_disable_irq() {
	arch_cli();
}

void kh_enable_irq() {
	arch_sti();
}

void hali_set_sched_timer() {
	arch_write_lapic(hali_lapic_vbase, ARCH_LAPIC_REG_LVT_TIMER, 0x30 | ARCH_LAPIC_LVT_TIMER_REG_PERIODIC);
	arch_write_lapic(hali_lapic_vbase, ARCH_LAPIC_REG_INITIAL_COUNT, hali_sched_interval);
}

void hali_div_err_isr_impl(
	uint64_t rdi,
	uint64_t rsi,
	uint64_t rdx,
	uint64_t rcx,
	uint64_t r8,
	uint64_t r9,

	const uint64_t saved_r11,
	const uint64_t saved_r10,
	const uint64_t saved_r9,
	const uint64_t saved_r8,
	const uint64_t saved_rdx,
	const uint64_t saved_rcx,
	const uint64_t saved_rax,

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
	if (!mm_is_user_space((void *)rip))
		km_panic("A kernel 0 division error has occurred at 0x%p", (void *)rip);
}

void hali_debug_isr_impl(
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

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
}

void hali_nmi_isr_impl() {
	km_panic("Non-maskable interrupt (NMI) is triggered");
}

void hali_breakpoint_isr_impl(
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

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
}

void hali_overflow_isr_impl(
	uint64_t rdi,
	uint64_t rsi,
	uint64_t rdx,
	uint64_t rcx,
	uint64_t r8,
	uint64_t r9,

	const uint64_t saved_r11,
	const uint64_t saved_r10,
	const uint64_t saved_r9,
	const uint64_t saved_r8,
	const uint64_t saved_rdx,
	const uint64_t saved_rcx,
	const uint64_t saved_rax,

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
	if (!mm_is_user_space((void *)rip))
		km_panic("A kernel integer overflow error has raised at 0x%p", (void *)rip);
	// TODO: Do anything to handle this situation.
}

void hali_bound_range_exceeded_isr_impl(
	uint64_t rdi,
	uint64_t rsi,
	uint64_t rdx,
	uint64_t rcx,
	uint64_t r8,
	uint64_t r9,

	const uint64_t saved_r11,
	const uint64_t saved_r10,
	const uint64_t saved_r9,
	const uint64_t saved_r8,
	const uint64_t saved_rdx,
	const uint64_t saved_rcx,
	const uint64_t saved_rax,

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
	if (!mm_is_user_space((void *)rip))
		km_panic("A kernel integer overflow error has raised at 0x%p", (void *)rip);
	// TODO: Do anything to handle this situation.
}

void hali_invalid_opcode_isr_impl(
	uint64_t rdi,
	uint64_t rsi,
	uint64_t rdx,
	uint64_t rcx,
	uint64_t r8,
	uint64_t r9,

	const uint64_t saved_r11,
	const uint64_t saved_r10,
	const uint64_t saved_r9,
	const uint64_t saved_r8,
	const uint64_t saved_rdx,
	const uint64_t saved_rcx,
	const uint64_t saved_rax,

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
	if (!mm_is_user_space((void *)rip))
		km_panic("Illegal kernel instruction at 0x%p was detected", (void *)rip);
}

void hali_nofpu_isr_impl(
	uint64_t rdi,
	uint64_t rsi,
	uint64_t rdx,
	uint64_t rcx,
	uint64_t r8,
	uint64_t r9,

	const uint64_t saved_r11,
	const uint64_t saved_r10,
	const uint64_t saved_r9,
	const uint64_t saved_r8,
	const uint64_t saved_rdx,
	const uint64_t saved_rcx,
	const uint64_t saved_rax,

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
	if (!mm_is_user_space((void *)rip))
		km_panic("Trying to invoke FPU in the kernel at 0x%p where FPU is not available", (void *)rip);
	// TODO: Do something to handle this.
}

void hali_double_fault_isr_impl(
	uint64_t rdi,
	uint64_t rsi,
	uint64_t rdx,
	uint64_t rcx,
	uint64_t r8,
	uint64_t r9,

	const uint64_t saved_r11,
	const uint64_t saved_r10,
	const uint64_t saved_r9,
	const uint64_t saved_r8,
	const uint64_t saved_rdx,
	const uint64_t saved_rcx,
	const uint64_t saved_rax,

	const uint64_t error_code,

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
	km_panic("Double fault is triggered at %p", rip);
}

void hali_fpu_segment_overrun_impl(
	uint64_t rdi,
	uint64_t rsi,
	uint64_t rdx,
	uint64_t rcx,
	uint64_t r8,
	uint64_t r9,

	const uint64_t saved_r11,
	const uint64_t saved_r10,
	const uint64_t saved_r9,
	const uint64_t saved_r8,
	const uint64_t saved_rdx,
	const uint64_t saved_rcx,
	const uint64_t saved_rax,

	const uint64_t error_code,

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
	if (!mm_is_user_space((void *)rip))
		km_panic("Triggered interrupt #9 unexpectedly in the kernel at 0x%p", (void *)rip);
	// TODO: Handle this.
}

void hali_invalid_tss_isr_impl(
	uint64_t rdi,
	uint64_t rsi,
	uint64_t rdx,
	uint64_t rcx,
	uint64_t r8,
	uint64_t r9,

	const uint64_t saved_r11,
	const uint64_t saved_r10,
	const uint64_t saved_r9,
	const uint64_t saved_r8,
	const uint64_t saved_rdx,
	const uint64_t saved_rcx,
	const uint64_t saved_rax,

	const uint64_t error_code,

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
	if (!mm_is_user_space((void *)rip))
		km_panic("Invalid TSS exception has triggered in kernel at 0x%p", (void *)rip);
}

void hali_seg_not_present_isr_impl(
	uint64_t rdi,
	uint64_t rsi,
	uint64_t rdx,
	uint64_t rcx,
	uint64_t r8,
	uint64_t r9,

	const uint64_t saved_r11,
	const uint64_t saved_r10,
	const uint64_t saved_r9,
	const uint64_t saved_r8,
	const uint64_t saved_rdx,
	const uint64_t saved_rcx,
	const uint64_t saved_rax,

	const uint64_t error_code,

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
	if (!mm_is_user_space((void *)rip))
		km_panic("Segment not present error has triggered in kernel at 0x%p", (void *)rip);
}

void hali_stack_seg_fault_isr_impl(
	uint64_t rdi,
	uint64_t rsi,
	uint64_t rdx,
	uint64_t rcx,
	uint64_t r8,
	uint64_t r9,

	const uint64_t saved_r11,
	const uint64_t saved_r10,
	const uint64_t saved_r9,
	const uint64_t saved_r8,
	const uint64_t saved_rdx,
	const uint64_t saved_rcx,
	const uint64_t saved_rax,

	const uint64_t error_code,

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
}

void hali_general_protect_isr_impl(
	uint64_t rdi,
	uint64_t rsi,
	uint64_t rdx,
	uint64_t rcx,
	uint64_t r8,
	uint64_t r9,

	const uint64_t saved_r11,
	const uint64_t saved_r10,
	const uint64_t saved_r9,
	const uint64_t saved_r8,
	const uint64_t saved_rdx,
	const uint64_t saved_rcx,
	const uint64_t saved_rax,

	const uint64_t error_code,

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
	if (!mm_is_user_space((void *)rip))
		km_panic("General protection error has triggered in kernel at 0x%p", (void *)rip);
}

void hali_pgfault_isr_impl() {
	// TODO: Implement it.
}

void hali_fpu_except_isr_impl() {
}

void hali_align_check_isr_impl() {
}

void hali_machine_check_isr_impl() {
}

void hali_simd_except_isr_impl() {
}

void hali_virt_except_isr_impl() {
}

void hali_ctrl_protect_isr_impl() {
}

void hali_hypervisor_inject_except_isr_impl() {
}

void hali_vmm_comm_except_isr_impl() {
}

void hali_security_err_isr_impl() {
}

void hali_usable_irq_isr_impl(
	const uint64_t irq_id,
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

	const uint64_t rip,
	const uint64_t cs,
	const uint64_t rflags,
	const uint64_t rsp,
	const uint64_t ss) {
}

PBOS_EXTERN_C_END
