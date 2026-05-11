#ifndef _HAL_X86_64_PROC_H_
#define _HAL_X86_64_PROC_H_

#include <pbos/kf/list.h>
#include <pbos/kf/rbtree.h>
#include <pbos/ki/km/proc.hh>
#ifdef __cplusplus
	#include "mm.hh"
#else
	#include "mm.h"
#endif

PBOS_EXTERN_C_BEGIN

typedef struct _kh_user_context_t {
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rbp;
	uint64_t rsp;
	uint64_t rsp0;
	uint64_t rbx;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rax;
	void *rip;					  // RIP
	uint64_t rflags;			  // RFLAGS
	uint64_t dr0, dr3, dr6, dr7;  // DR[0-367]
	uint16_t cs, ds, ss, es, gs;  // Selectors
} kh_user_context_t;

extern void *hn_lapic_pbase;
extern uint32_t *hn_lapic_vbase;
extern uint32_t hn_sched_interval;

// #define PS_TCB_SCHEDULED 0x01

kh_user_context_t *ps_alloc_context();
void ps_destroy_context(kh_user_context_t *context);

void ps_save_context(kh_user_context_t *ctxt);
PBOS_NORETURN void ps_load_user_context(const kh_user_context_t *ctxt);
PBOS_NORETURN void ps_load_kernel_context(const kh_user_context_t *ctxt);

PBOS_EXTERN_C_END

#endif
