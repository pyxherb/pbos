#ifndef _HAL_I386_PROC_H_
#define _HAL_I386_PROC_H_

#include <oicos/kf/list.h>
#include <oicos/kf/rbtree.h>
#include <oicos/km/objmgr.h>
#include <oicos/kn/km/proc.h>
#include "mm.h"

typedef struct _hn_proc_context_t {
	struct {
		uint32_t edi;
		uint32_t esi;
		uint32_t ebp;
		uint32_t esp;
		uint32_t ebx;
		uint32_t edx;
		uint32_t ecx;
		uint32_t eax;
	} gp_regs;
	void *eip;						  // EIP
	uint32_t eflags;				  // EFLAGS
	uint16_t cs, ds, ss, es, fs, gs;  // Segment registers
	uint32_t cr0, cr2, cr4;			  // CR[024]
	uint32_t xcr0;					  // XCR0
	uint32_t dr0, dr3, dr6, dr7;	  // DR[0-367]
	uint32_t tr6, tr7;				  // TR[6-7]
} hn_proc_context_t;

#define PROC_P 0x01	 // Present
#define PROC_A 0x02	 // Available

/// @brief Page Allocation Registry for Process (PARP)
typedef struct _hn_parp_t {
	kf_rbtree_node_t tree_header;
	pgaddr_t addr : 20;
	uint8_t order : 4;
	uint8_t flags : 8;
} hn_parp_t;

/// @brief Process Context Block (PCB)
typedef struct _ps_pcb_t {
	om_object_t object_header;

	kf_rbtree_t parp_list;
	mm_context_t *mmctxt;
	ps_tcb_t *threads;
	uint8_t priority, flags;
} ps_pcb_t;

/// @brief Thread Information Block (TIB)
typedef struct _ps_tcb_t {
	kf_list_node_t list_header;
	om_object_t object_header;

	ps_pcb_t *parent;

	uint8_t priority, flags;

	hn_proc_context_t context;
	pgaddr_t stack : 20;
	pgsize_t stacksize : 20;
} ps_tcb_t;

extern ps_pcb_t *hn_proc_list;

void hn_proc_cleanup(ps_pcb_t *proc);
void hn_thread_cleanup(ps_tcb_t *thread);
void hn_save_context(hn_proc_context_t *ctxt);

#endif
