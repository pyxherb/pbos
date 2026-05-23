#include <arch/x86_64/apic.h>
#include <pbos/fs/file.h>
#include <pbos/fs/fs.h>
#include <hal/x86_64/proc.hh>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/allocator.hh>
#include <pbos/kfxx/scope_guard.hh>

PBOS_EXTERN_C_BEGIN

ps_pcb_t **ps_cur_proc_per_cpu;
ps_tcb_t **ps_cur_thread_per_cpu;

void kh_yield_cur_thread() {
	ps_tcb_t *tcb = ps_get_cur_thread();
	tcb->scheduled = false;
	arch_write_lapic(hali_lapic_vbase, ARCH_LAPIC_REG_LVT_TIMER, 0x30 | ARCH_LAPIC_LVT_TIMER_REG_PERIODIC);
	arch_write_lapic(hali_lapic_vbase, ARCH_LAPIC_REG_INITIAL_COUNT, 0);
	while (tcb->scheduled);
}

void kh_switch_to_user_process(ps_pcb_t *pcb) {
	mm_switch_context(pcb->mm_context);
}

void kh_switch_to_user_thread(ps_tcb_t *tcb) {
	ps_load_user_context(tcb->context);
}

void kh_switch_to_kernel_thread(ps_tcb_t *tcb) {
	ps_load_kernel_context(tcb->context);
}

ps_cpuid_t kh_get_cur_cpuid() {
	return arch_storefs();
}

void kh_set_cur_cpuid(ps_cpuid_t cpuid) {
	arch_loadfs(cpuid);
}

PBOS_EXTERN_C_END
