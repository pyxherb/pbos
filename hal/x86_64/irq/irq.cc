#include <hal/x86_64/irq.h>
#include <hal/x86_64/syscall.h>
#include <pbos/km/logger.h>
#include <hal/x86_64/proc.hh>
#include <arch/x86_64/apic.h>

PBOS_EXTERN_C_BEGIN

hal_irq_context_t **irq_contexts;

bool hal_is_irq_disabled() {
	return !(arch_rflags() & (1 << 9));
}

void hal_disable_irq() {
	arch_cli();
}

void hal_enable_irq() {
	arch_sti();
}

void hn_set_sched_timer() {
	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_LVT_TIMER, 0x30 | ARCH_LAPIC_LVT_TIMER_REG_PERIODIC);
	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_INITIAL_COUNT, hn_sched_interval);
}

PBOS_EXTERN_C_END
