#include <arch/x86_64/kargs.h>
#include <arch/x86_64/misc.h>
#include <pbos/hal/init.h>
#include <pbos/ps/exec.h>
#include <pbos/km/panic.h>
#include <pbos/ki/ps/proc.hh>
#include "irq.hh"
#include "logger.hh"
#include "misc.h"
#include "mm.hh"

PBOS_EXTERN_C_BEGIN

void hal_init() {
	if (LIMINE_BASE_REVISION_SUPPORTED(hali_limine_base_revision) == false) {
		// TODO: Panic
		arch_hlt();
	}

	mm_kernel_bottom_mapping_base_vaddr = (void *)hali_limine_hhdm_request.response->offset;
}

PBOS_EXTERN_C_END
