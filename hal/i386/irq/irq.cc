#include <hal/i386/irq.h>
#include <hal/i386/proc.h>
#include <hal/i386/syscall.h>
#include <pbos/km/logger.h>

PBOS_EXTERN_C_BEGIN

hal_irq_context_t **hal_irq_contexts;

void hn_set_sched_timer() {
	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_LVT_TIMER, 0x30 & ~ARCH_LAPIC_LVT_TIMER_REG_MASKED | ARCH_LAPIC_LVT_TIMER_REG_PERIODIC);
	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_INITIAL_COUNT, hn_sched_interval);
}

PBOS_NORETURN void isr_timer_impl(
	const uint32_t eax,
	const uint32_t ebx,
	const uint32_t ecx,
	const uint32_t edx,
	const uint32_t esi,
	const uint32_t edi,
	const uint32_t ebp,
	const uint32_t eip,
	const uint32_t cs,
	const uint32_t eflags,
	const uint32_t esp) {
	arch_cli();

	ps_euid_t cur_euid = ps_get_cur_euid();
	ps_pcb_t *cur_proc = ps_get_cur_proc();
	ps_tcb_t *cur_thread = ps_get_cur_thread();
	ps_tcb_t *next_thread = ps_cur_sched->next_thread(ps_cur_sched, cur_euid, cur_proc, cur_thread);

	if (next_thread->parent != cur_proc) {
		kn_switch_to_user_process(next_thread->parent);
		ps_cur_proc_per_eu[cur_euid] = next_thread->parent;
	}

	ps_cur_thread_per_eu[cur_euid] = next_thread;

	next_thread->flags |= PS_TCB_SCHEDULED;

	if (cur_thread) {
		cur_thread->context->eax = eax;
		cur_thread->context->ebx = ebx;
		cur_thread->context->ecx = ecx;
		cur_thread->context->edx = edx;
		cur_thread->context->esi = esi;
		cur_thread->context->edi = edi;
		cur_thread->context->esp = esp;
		cur_thread->context->ebp = ebp;
		cur_thread->context->eip = (void *)eip;
		cur_thread->context->eflags = eflags;
	}

	hn_set_sched_timer();

	arch_sti();

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_EOI, 0);

	kn_switch_to_user_thread(next_thread);
}

PBOS_EXTERN_C_END
