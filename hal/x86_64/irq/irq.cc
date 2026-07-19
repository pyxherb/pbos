#include <hal/x86_64/irq.hh>
#include <pbos/kd/logger.h>
#include <hal/x86_64/proc.hh>
#include <arch/x86_64/apic.h>

PBOS_EXTERN_C_BEGIN

alignas(16) arch_gate_t hali_kidt[512] = {};

size_t kh_get_irq_max() {
	return PBOS_ARRAYSIZE(hali_kidt) / 2;
}

void hali_set_isr(hali_isr_t isr, size_t irq, uint8_t dpl, uint8_t gate_type) {
	kd_assert(irq <= PBOS_ARRAYSIZE(hali_kidt) / 2);
	kd_assert(dpl <= 3);
	hali_kidt[(irq * 2)] = GATEDESC_LOW(isr, SELECTOR_KCODE, GATEDESC_ATTRIBS(1, dpl, 0, gate_type));
	hali_kidt[(irq * 2) + 1] = GATEDESC_HIGH(isr);
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

PBOS_EXTERN_C_END
