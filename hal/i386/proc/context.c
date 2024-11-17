#include <arch/i386/reg.h>
#include <hal/i386/proc.h>
#include <pbos/km/logger.h>

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

PB_NORETURN void hn_load_user_context();

void ps_save_context(ps_user_context_t *ctxt) {
}

PB_NORETURN void ps_load_user_context(ps_user_context_t *ctxt) {
	kdprintf("Switching context:\n"
			"EAX=%.8x EBX=%.8x\n"
			"ECX=%.8x EDX=%.8x\n"
			"ESP=%.8x EBP=%.8x\n"
			"ESI=%.8x EDI=%.8x\n"
			"EIP=%.8x EFLAGS=%.8x\n",
			ctxt->eax, ctxt->ebx,
			ctxt->ecx, ctxt->edx,
			ctxt->esp, ctxt->ebp,
			ctxt->esi, ctxt->edi,
			ctxt->eip, ctxt->eflags);
	hn_ctxtsw_tmp_area->cs = SELECTOR_UCODE;
	hn_ctxtsw_tmp_area->ds = SELECTOR_UDATA;
	hn_ctxtsw_tmp_area->es = SELECTOR_UDATA;
	hn_ctxtsw_tmp_area->fs = ps_get_current_euid();
	hn_ctxtsw_tmp_area->gs = SELECTOR_UDATA;
	memcpy(&hn_ctxtsw_tmp_area->context, ctxt, sizeof(ps_user_context_t));
	hn_load_user_context();
}
