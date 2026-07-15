#include <pbkxrt/init.h>
#include <pbos/dm/device.h>
#include <pbos/kd/logger.h>
#include <pbos/pci/driver.h>
#include <pbos/iodev/kbd.h>
#include "common.h"

PBOS_EXTERN_C_BEGIN

PBOS_USED PBOS_KMOD_API char PBOS_MODULE_NAME[] = PCI_PCIBUS_KMOD_NAME;

PBOS_USED PBOS_KMOD_API char PBOS_MODULE_DEPS[] =
	IODEV_KBD_KMOD_NAME;

PBOS_USED PBOS_KMOD_API km_result_t pbos_module_init() {
	kxi_call_ctors();

	return KM_RESULT_OK;
}

PBOS_USED PBOS_KMOD_API void pbos_module_deinit() {
	kxi_call_dtors();
}

PBOS_EXTERN_C_END
