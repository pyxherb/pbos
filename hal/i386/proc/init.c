#include <hal/i386/proc.h>
#include <pbos/kn/km/exec.h>
#include <arch/i386/io.h>
#include <arch/i386/irq.h>

void hal_prepare_ps() {
	for (size_t i = 0; i < KCTXTSWTMP_SIZE; i += PAGESIZE) {
		void *paddr = mm_pgalloc(KN_PMEM_AVAILABLE);
		if (!paddr)
			km_panic("Error allocating memory for user context area");
		if (KM_FAILED(mm_mmap(mm_kernel_context, (void *)(KCTXTSWTMP_VBASE + i), paddr, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE | PAGE_USER, 0)))
			km_panic("Error mapping the user context area");
	}

	if (!(ps_cur_proc_per_eu = mm_kmalloc(ps_eu_num * sizeof(ps_pcb_t *)))) {
		km_panic("Unable to allocate current PCB pointer storage for all CPUs");
	}

	memset(ps_cur_proc_per_eu, 0, ps_eu_num * sizeof(ps_pcb_t *));

	if (!(ps_cur_thread_per_eu = mm_kmalloc(ps_eu_num * sizeof(ps_tcb_t *)))) {
		km_panic("Unable to allocate current TCB pointer storage for all CPUs");
	}

	memset(ps_cur_thread_per_eu, 0, ps_eu_num * sizeof(ps_tcb_t *));
}

PB_NORETURN void kn_enter_sched_halt();

PB_NORETURN void kn_enter_sched(ps_euid_t euid) {
	arch_loadfs(euid);
	arch_sti();
	static uint16_t COUNT_RATE = 11931;
	arch_out8(0x40, (COUNT_RATE) & 0xff);
	arch_out8(0x40, (COUNT_RATE >> 8) & 0xff);
	kn_enter_sched_halt();
}
