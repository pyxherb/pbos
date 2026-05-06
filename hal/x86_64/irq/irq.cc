#include <hal/x86_64/irq.hh>
#include <hal/x86_64/ps/syscall.hh>
#include <pbos/km/logger.h>
#include <hal/x86_64/proc.hh>
#include <arch/x86_64/apic.h>

PBOS_EXTERN_C_BEGIN

hal_irq_context_t **irq_contexts;

bool irq_is_disabled() {
	return !(arch_rflags() & (1 << 9));
}

void irq_disable() {
	arch_cli();
}

void irq_enable() {
	arch_sti();
}

void hn_set_sched_timer() {
	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_LVT_TIMER, 0x30 | ARCH_LAPIC_LVT_TIMER_REG_PERIODIC);
	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_INITIAL_COUNT, hn_sched_interval);
}

PBOS_EXTERN_C_END
