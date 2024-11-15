#ifndef _PBOS_KM_PROC_H_
#define _PBOS_KM_PROC_H_

#include <pbos/generated/km.h>
#include "mm.h"
#include "objmgr.h"

#define PROC_ACCESS_EXEC 0x00000001		 // Execute other programs
#define PROC_ACCESS_PRIORITY 0x00000002	 // Set priority
#define PROC_ACCESS_LOAD 0x00000004		 // Load modules
#define PROC_ACCESS_ADVMM 0x00000008	 // Advanced memory management
#define PROC_ACCESS_POWER 0x00000010	 // Power management
#define PROC_ACCESS_DBG 0x00000020		 // Debugging
#define PROC_ACCESS_SESSION 0x00000040	 // Session
#define PROC_ACCESS_NET 0x00000080		 // Network

typedef struct _ps_mod_t ps_mod_t;
typedef struct _ps_pcb_t ps_pcb_t;	// Process Environment Descriptor (PED)
typedef struct _ps_tcb_t ps_tcb_t;	// Thread Environment Descriptor (TED)
typedef struct _ps_user_context_t ps_user_context_t;

typedef uint32_t ps_proc_access_t;

#define PM_PROC_ID_MAX PROC_MAX
#define PM_THREAD_ID_MAX UINT32_MAX

typedef int32_t proc_id_t;
typedef int32_t thread_id_t;
typedef uint32_t euid_t;

typedef void (*thread_proc_t)(void *args);

extern om_class_t *ps_proc_class, *ps_thread_class;
extern uint32_t ps_eu_num;

#define PROC_CLASSID UUID(88e8f612, 0b0c, 4f75, 921b, 88110ca3b116)
#define THREAD_CLASSID UUID(5dd4ece1, 89a0, 4bec, b14b, 62e11312723d)

proc_id_t ps_create_proc(
	ps_proc_access_t access,
	proc_id_t parent);
thread_id_t ps_create_thread(
	ps_proc_access_t access,
	ps_pcb_t* pcb,
	size_t stacksize);

uint16_t ps_maxproc();

ps_pcb_t *ps_getpcb(proc_id_t pid);
proc_id_t *ps_getpid(ps_pcb_t *pcb);

/// @brief Get current executing process.
///
/// @return Current process object.
ps_pcb_t *ps_curproc();

/// @brief Get memory context of a process.
///
/// @param proc Target process object.
/// @return Memory context of the process.
mm_context_t *ps_mmcontext_of(ps_pcb_t *proc);

/// @brief Get current executing thread.
///
/// @return Current thread object.
ps_tcb_t *ps_curthread();

void ps_init();

void ps_add_thread(ps_pcb_t *proc, ps_tcb_t *thread);

euid_t ps_get_current_euid();
void kn_set_current_euid(euid_t euid);

#endif
