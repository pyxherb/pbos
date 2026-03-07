#ifndef _PBOS_KN_KM_PROC_HH_
#define _PBOS_KN_KM_PROC_HH_

#include <pbos/km/proc.hh>
#include <pbos/kfxx/rbtree.hh>

PBOS_EXTERN_C_BEGIN

extern kfxx::rbtree_t<proc_id_t> ps_global_proc_set;
extern ps_sched_t ps_simploop_sched;

void hal_prepare_ps();
void ps_init();

PBOS_NORETURN void kn_enter_sched(ps_euid_t euid);

void kn_proc_destructor(om_object_t *obj);
void kn_thread_destructor(om_object_t *obj);

proc_id_t kn_alloc_proc_id();

void ps_thread_set_entry(ps_tcb_t *tcb, void *ptr);
void kn_thread_set_stack(ps_tcb_t *tcb, void *ptr, size_t size);
void kn_thread_set_kernel_stack(ps_tcb_t *tcb, void *ptr, size_t size);

void kn_proc_addparp(ps_pcb_t *pcb, void *paddr, uint8_t order);
void kn_proc_delparp(ps_pcb_t *pcb, void *paddr, uint8_t order);

void kn_switch_to_user_process(ps_pcb_t *pcb);
PBOS_NORETURN void kn_switch_to_user_thread(ps_tcb_t *tcb);
PBOS_NORETURN void kn_switch_to_kernel_thread(ps_tcb_t *tcb);

PBOS_EXTERN_C_END

#endif
