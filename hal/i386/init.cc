#include <arch/i386/kargs.h>
#include <arch/i386/misc.h>
#include <pbos/hal/init.h>
#include <pbos/km/exec.h>
#include <pbos/km/mm.h>
#include <pbos/km/panic.h>
#include "initcar.h"
#include "irq.h"
#include "logger.h"
#include "misc.h"
#include <pbos/kn/km/proc.h>

PBOS_EXTERN_C_BEGIN

alignas(hn_ctor_t) hn_ctor_t KN_CTORS_BEGIN[0] = { };
alignas(hn_ctor_t) hn_ctor_t KN_CTORS_END[0] = { };

alignas(hn_dtor_t) hn_dtor_t KN_DTORS_BEGIN[0] = { };
alignas(hn_dtor_t) hn_dtor_t KN_DTORS_END[0] = { };

void hal_call_ctors() {
	kdprintf("Global ctor ptr: %p-%p\n", KN_CTORS_BEGIN, KN_CTORS_END);
	const size_t n_ctors = KN_CTORS_END - KN_CTORS_BEGIN;
	for (size_t i = 0; i < n_ctors; ++i) {
		kdprintf("Calling global ctor: %p\n", KN_CTORS_BEGIN[i]);
		KN_CTORS_BEGIN[i]();
	}
}

void hal_init() {
	hn_klog_init();

	if (ARCH_KARGS_PTR->magic[0] != KARG_MAGIC0 ||
		ARCH_KARGS_PTR->magic[1] != KARG_MAGIC1 ||
		ARCH_KARGS_PTR->magic[2] != KARG_MAGIC2 ||
		ARCH_KARGS_PTR->magic[3] != KARG_MAGIC3)
		km_panic("Kernel arguments damaged");

	kdprintf("Initializing global objects\n");

	hal_call_ctors();

	hn_mm_init();

	hal_irq_init();

	kdprintf("Initialized HAL\n");
}

PBOS_EXTERN_C_END
