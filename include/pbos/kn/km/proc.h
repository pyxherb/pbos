#ifndef _PBOS_KN_KM_PROC_H_
#define _PBOS_KN_KM_PROC_H_

#include <pbos/km/proc.h>

PBOS_EXTERN_C_BEGIN

extern ps_sched_t ps_simploop_sched;

void hal_prepare_ps();
void ps_init();

PBOS_NORETURN void kn_enter_sched(ps_euid_t euid);

void kn_proc_destructor(om_object_t *obj);
void kn_thread_destructor(om_object_t *obj);

proc_id_t kn_alloc_proc_id();

ps_pcb_t *kn_alloc_pcb();
ps_tcb_t *kn_alloc_tcb(ps_pcb_t* pcb);

km_result_t kn_thread_allocstack(ps_tcb_t *tcb, size_t size);

void kn_thread_setentry(ps_tcb_t *tcb, void *ptr);
void kn_thread_setstack(ps_tcb_t *tcb, void *ptr, size_t size);

void kn_proc_addparp(ps_pcb_t *pcb, void *paddr, uint8_t order);
void kn_proc_delparp(ps_pcb_t *pcb, void *paddr, uint8_t order);

void kn_switch_to_user_process(ps_pcb_t* pcb);
void kn_switch_to_user_thread(ps_tcb_t* tcb);

PBOS_EXTERN_C_END

#endif
