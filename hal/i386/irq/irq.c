#include <hal/i386/irq.h>
#include <hal/i386/proc.h>
#include <hal/i386/syscall.h>
#include <pbos/km/logger.h>

void isr_irq0_impl() {
	ps_euid_t cur_euid = ps_get_cur_euid();
	ps_pcb_t *cur_proc = ps_cur_procs[cur_euid];
	ps_tcb_t *cur_thread = ps_cur_threads[cur_euid];
	ps_tcb_t *next_thread;

	if (!cur_thread) {
		kprintf("Initial scheduling\n");

		ps_pcb_t *next_proc;
		next_proc = PB_CONTAINER_OF(ps_pcb_t, node_header, kf_rbtree_begin(&ps_global_proc_set));
		next_thread = PB_CONTAINER_OF(ps_tcb_t, node_header, kf_rbtree_begin(&next_proc->thread_set));
		ps_cur_procs[cur_euid] = next_proc;

		mm_switch_context(&next_proc->mmctxt);
	} else {
		kf_rbtree_node_t *next_thread_node = kf_rbtree_next(&cur_thread->node_header);

		kprintf("Scheduled EUID=%d\n", (int)cur_euid);

		if (!next_thread_node) {
			kf_rbtree_node_t *next_proc_node = kf_rbtree_next(&cur_proc->node_header);
			ps_pcb_t *next_proc;
			if (!next_proc_node) {
				kprintf("Rescheduling to the first process\n");
				next_proc = PB_CONTAINER_OF(ps_pcb_t, node_header, kf_rbtree_begin(&ps_global_proc_set));
			} else {
				kprintf("Rescheduling to the next process\n");
				next_proc = PB_CONTAINER_OF(ps_pcb_t, node_header, next_proc_node);
			}

			mm_switch_context(&next_proc->mmctxt);

			ps_cur_procs[cur_euid] = next_proc;
			next_thread = PB_CONTAINER_OF(ps_tcb_t, node_header, kf_rbtree_begin(&next_proc->thread_set));
		} else {
			kprintf("Rescheduling to the next thread\n");
			next_thread = PB_CONTAINER_OF(ps_tcb_t, node_header, next_thread_node);
		}
	}

	ps_cur_threads[cur_euid] = next_thread;

	if (next_thread != cur_thread) {
		next_thread->flags |= PS_TCB_SCHEDULED;

		if (cur_thread)
			ps_save_context(&cur_thread->context);

		if (next_thread->flags & PS_TCB_SCHEDULED) {
			next_thread->flags &= ~PS_TCB_SCHEDULED;
			arch_sti();

			static uint16_t COUNT_RATE = 11931;
			arch_out8(0x40, (COUNT_RATE) & 0xff);
			arch_out8(0x40, (COUNT_RATE >> 8) & 0xff);

			arch_out8(0x20, 0x20);

			kn_switch_to_user_thread(next_thread);
		}

		arch_sti();

		static uint16_t COUNT_RATE = 11931;
		arch_out8(0x40, (COUNT_RATE) & 0xff);
		arch_out8(0x40, (COUNT_RATE >> 8) & 0xff);

		arch_out8(0x20, 0x20);

	} else {
		static uint16_t COUNT_RATE = 11931;
		arch_out8(0x40, (COUNT_RATE) & 0xff);
		arch_out8(0x40, (COUNT_RATE >> 8) & 0xff);

		arch_out8(0x20, 0x20);
	}
}
