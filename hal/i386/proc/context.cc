#include <arch/i386/reg.h>
#include <pbos/hal/spinlock.h>
#include <pbos/km/logger.h>
#include <hal/i386/proc.hh>
#include <pbos/hal/irq.hh>

PBOS_EXTERN_C_BEGIN

typedef struct _kn_ctxtsw_tmp_t {
	uint32_t eax;
	uint32_t esp;
	uint32_t eflags;
	uint32_t eip;
	uint32_t cs;
} kn_ctxtsw_tmp_t;

PBOS_NORETURN void hn_load_user_context(
	uint32_t edi,
	uint32_t esi,
	uint32_t ebp,
	uint32_t ebx,
	uint32_t edx,
	uint32_t ecx,
	uint32_t fs,
	uint32_t es,
	uint32_t gs,
	uint32_t ds,
	uint32_t eax,

	void *eip,
	uint32_t cs,
	uint32_t eflags,
	uint32_t esp,
	uint32_t ss);

PBOS_NORETURN void hn_load_kernel_context(
	uint32_t edi,
	uint32_t esi,
	uint32_t ebp,
	uint32_t ebx,
	uint32_t edx,
	uint32_t ecx,
	uint32_t fs,
	uint32_t es,
	uint32_t gs,
	uint32_t ds,

	kn_ctxtsw_tmp_t *ctxtsw_tmp);

PBOS_NORETURN void ps_load_user_context(const ps_user_context_t *ctxt) {
	hn_load_user_context(
		ctxt->edi,
		ctxt->esi,
		ctxt->ebp,
		ctxt->ebx,
		ctxt->edx,
		ctxt->ecx,
		ps_get_cur_euid(),
		ctxt->es,
		ctxt->gs,
		ctxt->ds,
		ctxt->eax,
		ctxt->eip,
		ctxt->cs,
		ctxt->eflags,
		ctxt->esp,
		ctxt->ss);
}

PBOS_NORETURN void ps_load_kernel_context(const ps_user_context_t *ctxt) {
	kn_ctxtsw_tmp_t tmp = {
		.eax = ctxt->eax,
		.esp = ctxt->esp,
		.eflags = ctxt->eflags,
		.eip = (uint32_t)ctxt->eip,
		.cs = (uint32_t)ctxt->cs
	};

	kd_assert(ctxt->cs == SELECTOR_KCODE);

	hn_load_kernel_context(
		ctxt->edi,
		ctxt->esi,
		ctxt->ebp,
		ctxt->ebx,
		ctxt->edx,
		ctxt->ecx,
		ps_get_cur_euid(),
		ctxt->es,
		ctxt->gs,
		ctxt->ds,

		&tmp);
}

void ps_save_context(ps_user_context_t *ctxt) {
}

PBOS_EXTERN_C_END
