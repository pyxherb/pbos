#include <arch/i386/reg.h>
#include <pbos/hal/spinlock.h>
#include <pbos/km/logger.h>
#include <hal/i386/proc.hh>
#include <pbos/hal/irq.hh>

PBOS_EXTERN_C_BEGIN

typedef struct _kn_ctxtsw_tmp_t {
	ps_user_context_t context;
	struct {
		char padding[PAGESIZE - sizeof(ps_user_context_t)];
	};
	uint16_t cs;
	uint16_t ds;
	uint16_t es;
	uint16_t fs;
	uint16_t gs;
} kn_ctxtsw_tmp_t;

#define hn_ctxtsw_tmp_area ((kn_ctxtsw_tmp_t *)KCTXTSWTMP_VBASE)

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

PBOS_NORETURN void ps_load_user_context(ps_user_context_t *ctxt) {
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

void ps_save_context(ps_user_context_t *ctxt) {
}

PBOS_EXTERN_C_END
