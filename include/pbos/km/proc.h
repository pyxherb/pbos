#ifndef _PBOS_KM_PROC_H_
#define _PBOS_KM_PROC_H_

#include <pbos/fs/fs.h>
#include <pbos/generated/km.h>
#include <pbos/mm/mm.h>

PBOS_EXTERN_C_BEGIN

#define PS_INVALID_UHANDLE_VALUE UINT32_MAX

typedef uint32_t ps_proc_access_t;

typedef uint32_t ps_proc_id_t;
typedef uint32_t ps_thread_id_t;
typedef uint32_t ps_cpu_id_t;

typedef int ps_ufd_t;

// Process Control Block (PCB)
typedef struct _ps_pcb_t ps_pcb_t;

typedef struct _ps_ufcb_t ps_ufcb_t;

ps_ufd_t ps_alloc_fd(ps_pcb_t *pcb);
ps_ufcb_t *ps_alloc_ufcb(ps_pcb_t *pcb, fs_fcb_t *kernel_fcb, ps_ufd_t fd);
void ps_add_ufcb(ps_pcb_t *pcb, ps_ufcb_t *ufcb);
void ps_remove_ufcb(ps_pcb_t *pcb, ps_ufcb_t *ufcb);
ps_ufcb_t *ps_lookup_ufcb(ps_pcb_t *pcb, ps_ufd_t fd);

typedef struct _kh_user_context_t kh_user_context_t;

// Thread Control Block (TCB)
typedef struct _ps_tcb_t ps_tcb_t;

#define PM_PROC_ID_MAX PROC_MAX
#define PM_THREAD_ID_MAX UINT32_MAX

typedef void (*thread_proc_t)(void *args);

extern ps_pcb_t **ps_cur_proc_per_cpu;
extern ps_tcb_t **ps_cur_thread_per_cpu;

#define PS_PROC_P 0x01	// Present
#define PS_PROC_A 0x02	// Available

void ps_create_proc(
	ps_pcb_t *pcb,
	ps_proc_id_t parent);
ps_thread_id_t ps_create_thread(
	ps_proc_access_t access,
	ps_pcb_t *pcb,
	size_t stack_size);

uint16_t ps_maxproc();

ps_pcb_t *ps_getpcb(ps_proc_id_t pid);
ps_proc_id_t *ps_getpid(ps_pcb_t *pcb);

void ps_add_thread(ps_pcb_t *proc, ps_tcb_t *thread);

ps_pcb_t *ps_alloc_pcb();
ps_tcb_t *ps_alloc_tcb(ps_pcb_t *pcb);
km_result_t ps_thread_alloc_stack(ps_tcb_t *tcb, size_t size);
km_result_t ps_thread_alloc_kernel_stack(ps_tcb_t *tcb, size_t size);
mm_context_t *ps_mm_context_of(ps_pcb_t *pcb);

ps_pcb_t *ps_global_proc_set_begin();
ps_pcb_t *ps_global_proc_set_next(ps_pcb_t *cur);

ps_tcb_t *ps_proc_thread_set_begin(ps_pcb_t *pcb);
ps_tcb_t *ps_proc_thread_set_next(ps_pcb_t *pcb, ps_tcb_t *cur);

void ps_user_thread_init(ps_tcb_t *tcb);
void ps_thread_set_entry(ps_tcb_t *tcb, void *ptr);

ps_cpu_id_t ps_get_cur_cpuid();
void kn_set_cur_cpuid(ps_cpu_id_t cpuid);

ps_pcb_t *ps_get_cur_proc();
ps_tcb_t *ps_get_cur_thread();

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

PBOS_FORCEINLINE ps_sched_t *ps_get_sched() {
	return ps_cur_sched;
};
km_result_t ps_set_sched(ps_sched_t *sched);

PBOS_EXTERN_C_END

#endif
