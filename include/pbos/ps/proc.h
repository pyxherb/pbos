#ifndef _PBOS_PS_PROC_H_
#define _PBOS_PS_PROC_H_

#include <pbos/fs/fs.h>
#include <pbos/generated/km.h>
#include <pbos/mm/mm.h>

PBOS_EXTERN_C_BEGIN

#define PS_INVALID_UHANDLE_VALUE UINT32_MAX

typedef uint32_t ps_proc_access_t;

#define PS_PROC_ID_MAX UINT32_MAX
#define PS_THREAD_ID_MAX UINT32_MAX
#define PS_CPU_ID_MAX UINT32_MAX

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

fs_fcb_t *ps_kfcb_of_ufcb(ps_ufcb_t *ufcb);

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

PBOS_NODISCARD km_result_t ps_create_proc(
	ps_pcb_t *pcb,
	ps_proc_id_t parent);
ps_thread_id_t ps_create_thread(
	ps_proc_access_t access,
	ps_pcb_t *pcb,
	size_t stack_size);

uint16_t ps_max_proc();

ps_pcb_t *ps_lookup_pcb(ps_proc_id_t pid);
ps_proc_id_t ps_pid_of(ps_pcb_t *pcb);

void ps_add_thread(ps_pcb_t *proc, ps_tcb_t *thread);

ps_pcb_t *ps_alloc_pcb();
ps_tcb_t *ps_alloc_tcb(ps_pcb_t *pcb);

km_result_t ps_thread_alloc_stack(ps_tcb_t *tcb, size_t size);
km_result_t ps_thread_alloc_kernel_stack(ps_tcb_t *tcb, size_t size);
mm_context_t *ps_mm_context_of(ps_pcb_t *pcb);

void ps_user_thread_init(ps_tcb_t *tcb);
void ps_thread_set_entry(ps_tcb_t *tcb, void *ptr);

ps_cpu_id_t ps_get_cur_cpuid();
void ki_set_cur_cpuid(ps_cpu_id_t cpuid);

PBOS_API ps_pcb_t *ps_get_cur_proc();
PBOS_API ps_tcb_t *ps_get_cur_thread();

PBOS_API fs_fnode_t *ps_get_cwd(ps_pcb_t *pcb);
PBOS_API void ps_set_cwd(ps_pcb_t *pcb, fs_fnode_t *cwd_node);
PBOS_API void ps_unset_cwd(ps_pcb_t *pcb);

PBOS_API void ps_yield_cur_thread();

PBOS_EXTERN_C_END

#endif
