#include <arch/x86_64/kargs.h>
#include <arch/x86_64/misc.h>
#include <pbos/hal/init.h>
#include <pbos/km/exec.h>
#include <pbos/km/panic.h>
#include <pbos/kn/km/proc.hh>
#include "initcar.hh"
#include "irq.hh"
#include "logger.hh"
#include "misc.h"
#include "mm.hh"

PBOS_EXTERN_C_BEGIN

alignas(hn_ctor_t) hn_ctor_t KN_CTORS_BEGIN[0] = {};
alignas(hn_ctor_t) hn_ctor_t KN_CTORS_END[0] = {};

alignas(hn_dtor_t) hn_dtor_t KN_DTORS_BEGIN[0] = {};
alignas(hn_dtor_t) hn_dtor_t KN_DTORS_END[0] = {};

extern "C" void __cxa_pure_virtual() {
	km_panic("Attempting to call a pure virtual function!");
}

void hal_call_ctors() {
	kd_printf("Global ctor ptr: %p-%p\n", KN_CTORS_BEGIN, KN_CTORS_END);
	const size_t n_ctors = KN_CTORS_END - KN_CTORS_BEGIN;
	for (size_t i = 0; i < n_ctors; ++i) {
		kd_printf("Calling global ctor: %p\n", KN_CTORS_BEGIN[i]);
		KN_CTORS_BEGIN[i]();
	}
}

void hal_init() {
	if (LIMINE_BASE_REVISION_SUPPORTED(hn_limine_base_revision) == false) {
		// TODO: Panic
		arch_halt();
	}

	mm_kernel_bottom_mapping_base_vaddr = (void *)hn_limine_hhdm_request.response->offset;

	hn_klog_init();

	hal_call_ctors();

	kd_printf("Initializing global objects\n");

	hn_mm_init();

	// kh_irq_init();

	kd_printf("Initialized HAL\n");
}

PBOS_EXTERN_C_END
