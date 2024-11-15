#include <arch/i386/kargs.h>
#include <arch/i386/misc.h>
#include <pbos/hal/init.h>
#include <pbos/km/exec.h>
#include <pbos/km/mm.h>
#include <pbos/km/panic.h>
#include "initcar.h"
#include "logger.h"
#include "irq.h"

void hal_init() {
	hn_klog_init();

	if (ARCH_KARGS_PTR->magic[0] != KARG_MAGIC0 ||
		ARCH_KARGS_PTR->magic[1] != KARG_MAGIC1 ||
		ARCH_KARGS_PTR->magic[2] != KARG_MAGIC2 ||
		ARCH_KARGS_PTR->magic[3] != KARG_MAGIC3)
		km_panic("Kernel arguments damaged");

	hn_mm_init();

	irq_init();

	kdprintf("Initialized HAL\n");
}
