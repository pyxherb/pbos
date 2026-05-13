#include <pbos/kh/ps/proc.h>
#include <string.h>
#include <pbos/ki/km/proc.hh>
#include <pbos/ki/km/symbol.hh>

PBOS_EXTERN_C_BEGIN

kfxx::rbtree_t<ps_proc_id_t> ps_global_proc_set;
ps_sched_t *ps_cur_sched = NULL;

PBOS_API ps_pcb_t *ps_get_cur_proc() {
	if (ps_cur_proc_per_cpu)
		return ps_cur_proc_per_cpu[ps_get_cur_cpuid()];
	return nullptr;
}

PBOS_API ps_tcb_t *ps_get_cur_thread() {
	if (ps_cur_thread_per_cpu)
		return ps_cur_thread_per_cpu[ps_get_cur_cpuid()];
	return nullptr;
}

PBOS_API mm_context_t *ps_mm_context_of(ps_pcb_t *pcb) {
	return pcb->mm_context;
}

PBOS_API fs_fnode_t *ps_get_cwd(ps_pcb_t *pcb) {
	return pcb->cur_dir.get();
}

PBOS_API void ps_set_cwd(ps_pcb_t *pcb, fs_fnode_t *cwd_node) {
	pcb->cur_dir = cwd_node;
}

PBOS_API void ps_unset_cwd(ps_pcb_t *pcb) {
	pcb->cur_dir.reset();
}

PBOS_API void ps_yield_cur_thread() {
	kh_yield_cur_thread();
}

PBOS_EXTERN_C_END
