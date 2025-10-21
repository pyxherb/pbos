#ifndef _PBOS_KM_PROC_H_
#define _PBOS_KM_PROC_H_

#include <pbos/fs/fs.h>
#include <pbos/generated/km.h>
#include "mm.h"
#include "objmgr.h"

PBOS_EXTERN_C_BEGIN

#define PROC_ACCESS_EXEC 0x00000001		 // Execute other programs
#define PROC_ACCESS_PRIORITY 0x00000002	 // Set priority
#define PROC_ACCESS_LOAD 0x00000004		 // Load modules
#define PROC_ACCESS_ADVMM 0x00000008	 // Advanced memory management
#define PROC_ACCESS_POWER 0x00000010	 // Power management
#define PROC_ACCESS_DBG 0x00000020		 // Debugging
#define PROC_ACCESS_SESSION 0x00000040	 // Session
#define PROC_ACCESS_NET 0x00000080		 // Network

#define PS_INVALID_UHANDLE_VALUE UINT32_MAX

typedef uint32_t ps_proc_access_t;

typedef int32_t proc_id_t;
typedef int32_t thread_id_t;
typedef uint16_t ps_euid_t;

typedef int ps_ufd_t;

// Process Control Block (PCB)
typedef struct _ps_pcb_t ps_pcb_t;

typedef struct _ps_ufcb_t ps_ufcb_t;

ps_ufd_t ps_alloc_fd(ps_pcb_t *pcb);
ps_ufcb_t *ps_alloc_ufcb(ps_pcb_t *pcb, fs_fcb_t *kernel_fcb, ps_ufd_t fd);
void ps_add_ufcb(ps_pcb_t *pcb, ps_ufcb_t *ufcb);
void ps_remove_ufcb(ps_pcb_t *pcb, ps_ufcb_t *ufcb);
ps_ufcb_t *ps_lookup_ufcb(ps_pcb_t *pcb, ps_ufd_t fd);

typedef struct _ps_user_context_t ps_user_context_t;

// Thread Control Block (TCB)
typedef struct _ps_tcb_t ps_tcb_t;

#define PM_PROC_ID_MAX PROC_MAX
#define PM_THREAD_ID_MAX UINT32_MAX

typedef void (*thread_proc_t)(void *args);

extern om_class_t *ps_proc_class, *ps_thread_class;
extern uint32_t ps_eu_num;
extern ps_pcb_t **ps_cur_proc_per_eu;
extern ps_tcb_t **ps_cur_thread_per_eu;

#define PROC_CLASSID UUID(88e8f612, 0b0c, 4f75, 921b, 88110ca3b116)
#define THREAD_CLASSID UUID(5dd4ece1, 89a0, 4bec, b14b, 62e11312723d)

void ps_create_proc(
	ps_pcb_t *pcb,
	proc_id_t parent);
thread_id_t ps_create_thread(
	ps_proc_access_t access,
	ps_pcb_t *pcb,
	size_t stacksize);

uint16_t ps_maxproc();

ps_pcb_t *ps_getpcb(proc_id_t pid);
proc_id_t *ps_getpid(ps_pcb_t *pcb);

void ps_add_thread(ps_pcb_t *proc, ps_tcb_t *thread);

ps_pcb_t *ps_alloc_pcb();
ps_tcb_t *ps_alloc_tcb(ps_pcb_t *pcb);
km_result_t ps_thread_allocstack(ps_tcb_t *tcb, size_t size);
mm_context_t *ps_mm_context_of(ps_pcb_t *pcb);

ps_pcb_t *ps_global_proc_set_begin();
ps_pcb_t *ps_global_proc_set_next(ps_pcb_t *cur);

ps_tcb_t *ps_proc_thread_set_begin(ps_pcb_t *pcb);
ps_tcb_t *ps_proc_thread_set_next(ps_pcb_t *pcb, ps_tcb_t *cur);

void ps_user_thread_init(ps_tcb_t *tcb);
void ps_thread_set_entry(ps_tcb_t *tcb, void *ptr);

ps_euid_t ps_get_cur_euid();
void kn_set_cur_euid(ps_euid_t euid);

#define ps_get_cur_proc() (ps_cur_proc_per_eu[ps_get_cur_euid()])
#define ps_get_cur_thread() (ps_cur_thread_per_eu[ps_get_cur_euid()])

typedef struct _ps_sched_t ps_sched_t;

typedef km_result_t (*ps_sched_init_t)(ps_sched_t *sched);
typedef void (*ps_sched_deinit_t)(ps_sched_t *sched);
typedef km_result_t (*ps_sched_prepare_proc_t)(ps_sched_t *sched, ps_pcb_t *proc);
typedef km_result_t (*ps_sched_prepare_thread_t)(ps_sched_t *sched, ps_tcb_t *thread);
typedef void (*ps_sched_drop_proc_t)(ps_sched_t *sched, ps_pcb_t *proc);
typedef void (*ps_sched_drop_thread_t)(ps_sched_t *sched, ps_tcb_t *thread);
typedef ps_tcb_t *(*ps_sched_next_thread_t)(ps_sched_t *sched, ps_euid_t cur_euid, ps_pcb_t *cur_proc, ps_tcb_t *cur_thread);

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
