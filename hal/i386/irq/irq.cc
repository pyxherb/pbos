#include <hal/i386/irq.h>
#include <hal/i386/syscall.h>
#include <pbos/km/logger.h>
#include <hal/i386/proc.hh>

PBOS_EXTERN_C_BEGIN

hal_irq_context_t **hal_irq_contexts;

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

void isr_timer_impl(
	const uint32_t *const user_esp_ptr,
	const uint32_t eax,
	const uint32_t ebx,
	const uint32_t ecx,
	const uint32_t edx,
	const uint32_t esi,
	const uint32_t edi,
	const uint32_t ebp,
	const uint32_t gs,
	const uint32_t fs,
	const uint32_t es,
	const uint32_t ds,

	const uint32_t eip,
	const uint32_t cs,
	const uint32_t eflags) {
	bool is_user = false;
	ps_euid_t cur_euid = ps_get_cur_euid();
	ps_pcb_t *cur_proc = ps_get_cur_proc();
	ps_tcb_t *cur_thread = ps_get_cur_thread();
	ps_tcb_t *next_thread = ps_cur_sched->next_thread(ps_cur_sched, cur_euid, cur_proc, cur_thread);

	if (cur_thread) {
		void *ip = (void *)eip;
		void *sp = (void *)user_esp_ptr;
		void *bp = (void *)ebp;

		cur_thread->context->eax = eax;
		cur_thread->context->ebx = ebx;
		cur_thread->context->ecx = ecx;
		cur_thread->context->edx = edx;
		cur_thread->context->esi = esi;
		cur_thread->context->edi = edi;
		cur_thread->context->ebp = ebp;
		cur_thread->context->eip = (void *)eip;
		cur_thread->context->eflags = eflags;
		if ((uintptr_t)eip < KERNEL_VBASE) {
			// SS (32-bits?)->ESP->EFLAGS->CS (32-bits)->EIP
			cur_thread->context->esp = (uintptr_t)*((user_esp_ptr));
			// kdprintf("USER!!!\n");

			cur_thread->context->ds = ds;
			cur_thread->context->ss = ds;
		} else {
			// EFLAGS->CS (32-bits)->EIP
			cur_thread->context->esp = (uint32_t)(user_esp_ptr);
			// asm volatile("xchg %bx, %bx");
			// kdprintf("KERNEL!!!\n");

			cur_thread->context->ds = ds;
			cur_thread->context->ss = ds;
		}

		cur_thread->context->cs = cs;
		cur_thread->context->es = es;
		cur_thread->context->gs = gs;
	}

	if (next_thread->parent != cur_proc) {
		kn_switch_to_user_process(next_thread->parent);
		ps_cur_proc_per_eu[cur_euid] = next_thread->parent;
	}

	ps_cur_thread_per_eu[cur_euid] = next_thread;

	next_thread->flags |= PS_TCB_SCHEDULED;

	hn_set_sched_timer();

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_EOI, 0);

	if ((uintptr_t)next_thread->context->eip < KERNEL_VBASE) {
		kn_switch_to_user_thread(next_thread);
	}
}

PBOS_EXTERN_C_END
