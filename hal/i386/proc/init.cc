#include <arch/i386/io.h>
#include <hal/i386/irq.h>
#include <hal/i386/proc.hh>
#include <pbos/kn/km/exec.hh>

PBOS_EXTERN_C_BEGIN

void hal_prepare_ps() {
	if (!(ps_cur_proc_per_eu = (ps_pcb_t **)mm_kmalloc(ps_eu_num * sizeof(ps_pcb_t *), alignof(ps_pcb_t *)))) {
		km_panic("Unable to allocate current PCB pointer storage for all CPUs");
	}

	memset(ps_cur_proc_per_eu, 0, ps_eu_num * sizeof(ps_pcb_t *));

	if (!(ps_cur_thread_per_eu = (ps_tcb_t **)mm_kmalloc(ps_eu_num * sizeof(ps_tcb_t *), alignof(ps_tcb_t)))) {
		km_panic("Unable to allocate current TCB pointer storage for all CPUs");
	}

	memset(ps_cur_thread_per_eu, 0, ps_eu_num * sizeof(ps_tcb_t *));
}

PBOS_NORETURN void kn_enter_sched_halt();

PBOS_NORETURN void kn_enter_sched(ps_euid_t euid) {
	arch_loadfs(euid);

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_DIVIDE_CONFIG, ARCH_LAPIC_DIVIDE_CONFIG_16);
	hn_set_sched_timer();

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_EOI, 0);

	arch_sti();

	kn_enter_sched_halt();
}

PBOS_EXTERN_C_END
