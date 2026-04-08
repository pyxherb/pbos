#include <arch/x86_64/reg.h>
#include <pbos/hal/spinlock.h>
#include <pbos/km/logger.h>
#include <hal/x86_64/proc.hh>
#include <pbos/hal/irq.hh>

PBOS_EXTERN_C_BEGIN

typedef struct _kn_ctxtsw_tmp_t {
	uint64_t rax;
	uint64_t rsp;
	uint64_t rflags;
	uint64_t rip;
	uint64_t cs;
} kn_ctxtsw_tmp_t;

PBOS_NORETURN void hn_load_user_context(
	uint64_t rdi,
	uint64_t rsi,
	uint64_t rbp,
	uint64_t rbx,
	uint64_t rdx,
	uint64_t rcx,
	uint64_t fs,
	uint64_t es,
	uint64_t gs,
	uint64_t ds,
	uint64_t rax,

	void *eip,
	uint64_t cs,
	uint64_t rflags,
	uint64_t rsp,
	uint64_t ss);

PBOS_NORETURN void hn_load_kernel_context(
	uint64_t rdi,
	uint64_t rsi,
	uint64_t rbp,
	uint64_t rbx,
	uint64_t rdx,
	uint64_t rcx,
	uint64_t fs,
	uint64_t es,
	uint64_t gs,
	uint64_t ds,

	kn_ctxtsw_tmp_t *ctxtsw_tmp);

PBOS_NORETURN void ps_load_user_context(const kh_user_context_t *ctxt) {
	hn_load_user_context(
		ctxt->rdi,
		ctxt->rsi,
		ctxt->rbp,
		ctxt->rbx,
		ctxt->rdx,
		ctxt->rcx,
		ps_get_cur_euid(),
		ctxt->es,
		ctxt->gs,
		ctxt->ds,
		ctxt->rax,
		ctxt->rip,
		ctxt->cs,
		ctxt->rflags,
		ctxt->rsp,
		ctxt->ss);
}

PBOS_NORETURN void ps_load_kernel_context(const kh_user_context_t *ctxt) {
	kn_ctxtsw_tmp_t tmp = {
		.rax = ctxt->rax,
		.rsp = ctxt->rsp,
		.rflags = ctxt->rflags,
		.rip = (uint64_t)ctxt->rip,
		.cs = (uint64_t)ctxt->cs
	};

	kd_assert(ctxt->cs == SELECTOR_KCODE);

	hn_load_kernel_context(
		ctxt->rdi,
		ctxt->rsi,
		ctxt->rbp,
		ctxt->rbx,
		ctxt->rdx,
		ctxt->rcx,
		ps_get_cur_euid(),
		ctxt->es,
		ctxt->gs,
		ctxt->ds,

		&tmp);
}

void ps_save_context(kh_user_context_t *ctxt) {
}

PBOS_EXTERN_C_END
