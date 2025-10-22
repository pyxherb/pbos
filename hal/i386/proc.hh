#ifndef _HAL_I386_PROC_H_
#define _HAL_I386_PROC_H_

#include <pbos/kf/list.h>
#include <pbos/kf/rbtree.h>
#include <pbos/km/objmgr.h>
#include <pbos/kn/km/proc.hh>
#ifdef __cplusplus
	#include "mm.hh"
#else
	#include "mm.h"
#endif

PBOS_EXTERN_C_BEGIN

typedef struct _ps_user_context_t {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp;
	uint32_t esp0;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	void *eip;					  // EIP
	uint32_t eflags;			  // EFLAGS
	uint32_t dr0, dr3, dr6, dr7;  // DR[0-367]
	uint16_t cs, ds, ss, es, gs;  // Selectors
} ps_user_context_t;

#define PROC_P 0x01	 // Present
#define PROC_A 0x02	 // Available

/// @brief Page Allocation Registry for Process (PARP)
typedef struct _hn_parp_t {
	kf_rbtree_node_t tree_header;
	pgaddr_t addr : 20;
	uint8_t order : 4;
	uint8_t flags : 8;
} hn_parp_t;

#define PS_TCB_SCHEDULED 0x01

void hn_proc_cleanup(ps_pcb_t *proc);
void hn_thread_cleanup(ps_tcb_t *thread);
void ps_save_context(ps_user_context_t *ctxt);
PBOS_NORETURN void ps_load_user_context(const ps_user_context_t *ctxt);
PBOS_NORETURN void ps_load_kernel_context(const ps_user_context_t *ctxt);

PBOS_EXTERN_C_END

#endif
