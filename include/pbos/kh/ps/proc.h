#ifndef _PBOS_KH_PS_PROC_H_
#define _PBOS_KH_PS_PROC_H_

#include <pbos/ps/proc.h>

PBOS_EXTERN_C_BEGIN

void kh_switch_to_user_process(ps_pcb_t *pcb);
void kh_switch_to_user_thread(ps_tcb_t *tcb);
void kh_switch_to_kernel_thread(ps_tcb_t *tcb);
ps_cpu_id_t kh_get_cur_cpuid();
void kh_set_cur_cpuid(ps_cpu_id_t cpuid);

void kh_yield_cur_thread();

PBOS_EXTERN_C_END

#endif
