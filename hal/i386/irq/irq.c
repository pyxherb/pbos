#include <hal/i386/irq.h>
#include <hal/i386/proc.h>
#include <hal/i386/syscall.h>
#include <pbos/km/logger.h>

PB_NORETURN void isr_irq0_impl(
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
	ps_euid_t cur_euid = ps_get_cur_euid();
	ps_pcb_t *cur_proc = ps_get_cur_proc();
	ps_tcb_t *cur_thread = ps_get_cur_thread();
	ps_tcb_t *next_thread;

	if (!cur_thread) {
		ps_pcb_t *next_proc;
		next_proc = PB_CONTAINER_OF(ps_pcb_t, node_header, kf_rbtree_begin(&ps_global_proc_set));
		next_thread = PB_CONTAINER_OF(ps_tcb_t, node_header, kf_rbtree_begin(&next_proc->thread_set));
		ps_cur_proc_per_eu[cur_euid] = next_proc;

		kn_switch_to_user_process(next_proc);
	} else {
		kf_rbtree_node_t *next_thread_node = kf_rbtree_next(&cur_thread->node_header);

		if (!next_thread_node) {
			kf_rbtree_node_t *next_proc_node = kf_rbtree_next(&cur_proc->node_header);
			ps_pcb_t *next_proc;
			if (!next_proc_node) {
				next_proc = PB_CONTAINER_OF(ps_pcb_t, node_header, kf_rbtree_begin(&ps_global_proc_set));
			} else {
				next_proc = PB_CONTAINER_OF(ps_pcb_t, node_header, next_proc_node);
			}

			kn_switch_to_user_process(next_proc);

			ps_cur_proc_per_eu[cur_euid] = next_proc;
			next_thread = PB_CONTAINER_OF(ps_tcb_t, node_header, kf_rbtree_begin(&next_proc->thread_set));
		} else {
			next_thread = PB_CONTAINER_OF(ps_tcb_t, node_header, next_thread_node);
		}
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

	static uint16_t COUNT_RATE = 11931;
	arch_out8(0x40, (COUNT_RATE) & 0xff);
	arch_out8(0x40, (COUNT_RATE >> 8) & 0xff);

	arch_out8(0x20, 0x20);

	kn_switch_to_user_thread(next_thread);
}
