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

PBOS_NORETURN void isr_timer_impl(
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

	const uint32_t *const esp_top) {
	bool is_user = false;
	ps_euid_t cur_euid = ps_get_cur_euid();
	ps_pcb_t *cur_proc = ps_get_cur_proc();
	ps_tcb_t *cur_thread = ps_get_cur_thread();
	ps_tcb_t *next_thread = ps_cur_sched->next_thread(ps_cur_sched, cur_euid, cur_proc, cur_thread);

	uint32_t eip = esp_top[0];
	uint32_t cs = esp_top[1];
	uint32_t eflags = esp_top[2];

	if (cur_thread) {
		cur_thread->context->eax = eax;
		cur_thread->context->ebx = ebx;
		cur_thread->context->ecx = ecx;
		cur_thread->context->edx = edx;
		cur_thread->context->esi = esi;
		cur_thread->context->edi = edi;
		cur_thread->context->ebp = ebp;
		cur_thread->context->eip = (void *)eip;
		cur_thread->context->eflags = eflags;
		if (((uintptr_t)eip) < KERNEL_VBASE) {
			// SS (32-bits?)->ESP->EFLAGS->CS (32-bits)->EIP
			cur_thread->context->esp = esp_top[3];
			// kd_assert(cur_thread->context->esp0 == (uint32_t)((char*)cur_thread->kernel_stack + cur_thread->kernel_stack_size));
			// kd_printf("U!\n");

			cur_thread->context->ds = ds;
			cur_thread->context->ss = esp_top[4];
		} else {
			// EFLAGS->CS (32-bits)->EIP
			cur_thread->context->esp = (uint32_t)(&esp_top[3]);
			cur_thread->context->esp0 = cur_thread->context->esp;
			// kd_printf("K!\n");

			cur_thread->context->ds = ds;
			cur_thread->context->ss = ds;
		}

		cur_thread->context->cs = cs;
		cur_thread->context->es = es;
		cur_thread->context->gs = gs;
	}

	kd_assert(next_thread);

	hn_tss_storage_ptr[cur_euid].esp0 = next_thread->context->esp0;

	if (next_thread->parent != cur_proc) {
		kn_switch_to_user_process(next_thread->parent);
		ps_cur_proc_per_eu[cur_euid] = next_thread->parent;
	}

	ps_cur_thread_per_eu[cur_euid] = next_thread;

	next_thread->flags |= PS_TCB_SCHEDULED;

	hn_set_sched_timer();

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_EOI, 0);

	if (((uintptr_t)next_thread->context->eip) < KERNEL_VBASE) {
		/*kd_printf(
			"PID=%d, EIP=%.8x, ESP=%.8x, ESP0=%.8x U\n",
			next_thread->parent->rb_value, next_thread->context->eip, next_thread->context->esp, next_thread->context->esp0);*/

		kn_switch_to_user_thread(next_thread);
	} else {
		// We are already in ring 0.
		/*kd_printf(
			"PID=%d, EIP=%.8x, ESP=%.8x, ESP0=%.8x K\n",
			next_thread->parent->rb_value, next_thread->context->eip, next_thread->context->esp, next_thread->context->esp0);*/
		// asm volatile("xchg %bx, %bx");
		// asm volatile("xchg %bx, %bx");
		kn_switch_to_kernel_thread(next_thread);
	}
}

PBOS_EXTERN_C_END
