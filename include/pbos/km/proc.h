#ifndef _PBOS_KM_PROC_H_
#define _PBOS_KM_PROC_H_

#include <pbos/fs/fs.h>
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

typedef uint32_t ps_uhandle_t;

#define PS_INVALID_UHANDLE_VALUE UINT32_MAX

/// @brief User Handle Registry (UHR)
typedef struct _ps_uhr_t {
	om_object_t object_header;

	kf_rbtree_node_t node_header;
	om_object_t *kobject;
	ps_uhandle_t uhandle;
	size_t ref_num;
} ps_uhr_t;

typedef uint32_t ps_proc_access_t;

typedef int32_t proc_id_t;
typedef int32_t thread_id_t;
typedef uint16_t ps_euid_t;

typedef int ps_ufd_t;

// Process Control Block (PCB)
typedef struct _ps_pcb_t {
	kf_rbtree_node_t node_header;
	om_object_t object_header;

	proc_id_t proc_id;
	thread_id_t last_thread_id;
	kf_rbtree_t parp_list;
	mm_context_t *mm_context;
	kf_rbtree_t thread_set;
	uint8_t priority, flags;

	fs_file_t *cur_dir;

	kf_rbtree_t uhandle_map;
	ps_uhandle_t last_allocated_uhandle_value;

	kf_rbtree_t ufcontext_set;
	ps_ufd_t last_fd;
} ps_pcb_t;

typedef struct _ps_ufcontext_t {
	kf_rbtree_node_t node_header;
	fs_fcontext_t *kernel_fcontext;
	ps_ufd_t fd;
} ps_ufcontext_t;

ps_ufd_t ps_alloc_fd(ps_pcb_t *pcb);
ps_ufcontext_t *ps_alloc_ufcontext(ps_pcb_t *pcb, fs_fcontext_t *kernel_fcontext, ps_ufd_t fd);
void ps_add_ufcontext(ps_pcb_t *pcb, ps_ufcontext_t *ufcontext);
void ps_remove_ufcontext(ps_pcb_t *pcb, ps_ufcontext_t *ufcontext);
ps_ufcontext_t *ps_lookup_ufcontext(ps_pcb_t *pcb, ps_ufd_t fd);

typedef struct _ps_user_context_t ps_user_context_t;

// Thread Control Block (TCB)
typedef struct _ps_tcb_t {
	kf_rbtree_node_t node_header;
	om_object_t object_header;

	thread_id_t thread_id;
	ps_pcb_t *parent;

	uint8_t priority, flags;

	ps_user_context_t *context;
	void *stack;
	size_t stacksize;
} ps_tcb_t;

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

/// @brief Get current executing process.
///
/// @return Current process object.
ps_pcb_t *ps_curproc();

/// @brief Get current executing thread.
///
/// @return Current thread object.
ps_tcb_t *ps_curthread();

void ps_init();

void ps_add_thread(ps_pcb_t *proc, ps_tcb_t *thread);

ps_euid_t ps_get_cur_euid();
void kn_set_cur_euid(ps_euid_t euid);

#define ps_get_cur_proc() (ps_cur_proc_per_eu[ps_get_cur_euid()])
#define ps_get_cur_thread() (ps_cur_thread_per_eu[ps_get_cur_euid()])

km_result_t ps_create_uhandle(ps_pcb_t *proc, om_object_t *kobject, ps_uhandle_t *uhandle_out);
void ps_close_uhandle(ps_pcb_t *proc, ps_uhandle_t uhandle);
om_object_t *ps_lookup_uhandle(ps_pcb_t *proc, ps_uhandle_t uhandle);

#endif
