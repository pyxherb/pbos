#include <arch/x86_64/apic.h>
#include <pbos/km/logger.h>
#include <hal/x86_64/irq.hh>
#include <hal/x86_64/proc.hh>

PBOS_EXTERN_C_BEGIN

void hn_isr_timer_impl(
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

	const uint64_t *const rsp_top) {
	ps_cpu_id_t cur_cpuid = ps_get_cur_cpuid();
	ps_pcb_t *cur_proc = ps_get_cur_proc();
	ps_tcb_t *cur_thread = ps_get_cur_thread();
	ps_tcb_t *next_thread = ps_cur_sched->next_thread(ps_cur_sched, cur_cpuid, cur_proc, cur_thread);

	uint64_t rip = rsp_top[0];
	uint64_t cs = rsp_top[1];
	uint64_t rflags = rsp_top[2];

	if (cur_thread) {
		cur_thread->context->rax = rax;
		cur_thread->context->rbx = rbx;
		cur_thread->context->rcx = rcx;
		cur_thread->context->rdx = rdx;
		cur_thread->context->rsi = rsi;
		cur_thread->context->rdi = rdi;
		cur_thread->context->rbp = rbp;
		cur_thread->context->rip = (void *)rip;
		cur_thread->context->rflags = rflags;
		if (((uintptr_t)rip) < KSPACE_VBASE) {
			// SS (32-bits?)->ESP->EFLAGS->CS (32-bits)->EIP
			cur_thread->context->rsp = rsp_top[3];
			// kd_assert(cur_thread->context->esp0 == (uint32_t)((char*)cur_thread->kernel_stack + cur_thread->kernel_stack_size));
			// kd_printf("U!\n");

			cur_thread->context->ds = ds;
			cur_thread->context->ss = rsp_top[4];
		} else {
			// EFLAGS->CS (32-bits)->EIP
			cur_thread->context->rsp = (uint64_t)(rsp_top[3]);
			cur_thread->context->rsp0 = cur_thread->context->rsp;
			// kd_printf("K!\n");

			cur_thread->context->ds = ds;
			cur_thread->context->ss = ds;
		}

		cur_thread->context->cs = cs;
		cur_thread->context->es = es;
		cur_thread->context->gs = gs;
	}

	kd_assert(next_thread);

	hn_tss_storage_ptr[cur_cpuid].rsp0 = next_thread->context->rsp0;

	if (next_thread->parent != cur_proc) {
		ki_switch_to_user_process(next_thread->parent);
		ps_cur_proc_per_cpu[cur_cpuid] = next_thread->parent;
	}

	ps_cur_thread_per_cpu[cur_cpuid] = next_thread;

	next_thread->flags |= PS_TCB_SCHEDULED;

	hn_set_sched_timer();

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_EOI, 0);

	if (((uintptr_t)next_thread->context->rip) < KSPACE_VBASE) {
		/*kd_printf(
			"PID=%d, EIP=%.8x, ESP=%.8x, ESP0=%.8x U\n",
			next_thread->parent->rb_value, next_thread->context->eip, next_thread->context->esp, next_thread->context->esp0);*/

		ki_switch_to_user_thread(next_thread);
	} else {
		// We are already in ring 0.
		/*kd_printf(
			"PID=%d, EIP=%.8x, ESP=%.8x, ESP0=%.8x K\n",
			next_thread->parent->rb_value, next_thread->context->eip, next_thread->context->esp, next_thread->context->esp0);*/
		// asm volatile("xchg %bx, %bx");
		// asm volatile("xchg %bx, %bx");
		ki_switch_to_kernel_thread(next_thread);
	}
}

PBOS_EXTERN_C_END
