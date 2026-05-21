#ifndef _PBOS_PS_SCHED_H_
#define _PBOS_PS_SCHED_H_

#include "proc.h"

PBOS_EXTERN_C_BEGIN

PBOS_API ps_pcb_t *ps_global_proc_set_begin();
PBOS_API ps_pcb_t *ps_global_proc_set_next(ps_pcb_t *cur);

PBOS_API ps_tcb_t *ps_proc_thread_set_begin(ps_pcb_t *pcb);
PBOS_API ps_tcb_t *ps_proc_thread_set_next(ps_pcb_t *pcb, ps_tcb_t *cur);

typedef struct _ps_sched_t ps_sched_t;

typedef km_result_t (*ps_sched_init_t)(ps_sched_t *sched);
typedef void (*ps_sched_deinit_t)(ps_sched_t *sched);
typedef km_result_t (*ps_sched_prepare_proc_t)(ps_sched_t *sched, ps_pcb_t *proc);
typedef km_result_t (*ps_sched_prepare_thread_t)(ps_sched_t *sched, ps_tcb_t *thread);
typedef void (*ps_sched_drop_proc_t)(ps_sched_t *sched, ps_pcb_t *proc);
typedef void (*ps_sched_drop_thread_t)(ps_sched_t *sched, ps_tcb_t *thread);
typedef ps_tcb_t *(*ps_sched_next_thread_t)(ps_sched_t *sched, ps_cpu_id_t cur_cpuid, ps_pcb_t *cur_proc, ps_tcb_t *cur_thread);

typedef struct _ps_sched_t {
	ps_sched_init_t init;
	ps_sched_deinit_t deinit;
	ps_sched_prepare_proc_t prepare_proc;
	ps_sched_prepare_thread_t prepare_thread;
	ps_sched_drop_proc_t drop_proc;
	ps_sched_drop_thread_t drop_thread;
	ps_sched_next_thread_t next_thread;
} ps_sched_t;

extern ps_sched_t *ps_cur_sched;

PBOS_API ps_sched_t *ps_get_sched();
PBOS_API km_result_t ps_set_sched(ps_sched_t *sched);

PBOS_EXTERN_C_END

#endif
