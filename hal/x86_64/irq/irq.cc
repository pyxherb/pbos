#include <hal/x86_64/irq.hh>
#include <pbos/kd/logger.h>
#include <hal/x86_64/proc.hh>
#include <arch/x86_64/apic.h>

PBOS_EXTERN_C_BEGIN

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
