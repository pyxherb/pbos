#include <string.h>
#include <pbos/ki/km/proc.hh>
#include <pbos/ki/km/symbol.hh>

PBOS_EXTERN_C_BEGIN

kfxx::rbtree_t<ps_proc_id_t> ps_global_proc_set;
ps_sched_t *ps_cur_sched = NULL;

ps_pcb_t *ps_get_cur_proc() {
	return ps_cur_proc_per_cpu[ps_get_cur_cpuid()];
}
KI_EXPORT_IMAGE_SYMBOL(ps_get_cur_proc);

ps_tcb_t *ps_get_cur_thread() {
	return ps_cur_thread_per_cpu[ps_get_cur_cpuid()];
}
KI_EXPORT_IMAGE_SYMBOL(ps_get_cur_thread);

mm_context_t *ps_mm_context_of(ps_pcb_t *pcb) {
	return pcb->mm_context;
}
KI_EXPORT_IMAGE_SYMBOL(ps_mm_context_of);

fs_fnode_t *ps_get_cwd(ps_pcb_t *pcb) {
	return pcb->cur_dir.get();
}
KI_EXPORT_IMAGE_SYMBOL(ps_get_cwd);

void ps_set_cwd(ps_pcb_t *pcb, fs_fnode_t *cwd_node) {
	pcb->cur_dir = cwd_node;
}
KI_EXPORT_IMAGE_SYMBOL(ps_set_cwd);

void ps_unset_cwd(ps_pcb_t *pcb) {
	pcb->cur_dir.reset();
}

void ps_yield_cur_thread() {

}

PBOS_EXTERN_C_END
