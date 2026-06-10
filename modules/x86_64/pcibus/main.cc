#include <pbkxrt/init.h>
#include <pbos/kd/logger.h>
#include "mcfg.h"

PBOS_EXTERN_C_BEGIN

void pci_destroy_bus(dm_bus_t *bus) {
}

km_result_t pci_register_device(dm_bus_t *bus, dm_device_t *device) {
	return KM_RESULT_OK;
}

void pci_unregister_device(dm_bus_t *bus, dm_device_t *device) {
}

const dm_bus_ops_t pci_bus_ops = {
	.destroy_bus = pci_destroy_bus,
	.register_device = pci_register_device,
	.unregister_device = pci_unregister_device
};

PBOS_USED PBOS_KMOD_API char PBOS_MODULE_NAME[] = "pcibus";

PBOS_USED PBOS_KMOD_API km_result_t pbos_module_init() {
	kxi_call_ctors();

	pcibus_segment_group_id_to_domain_map = kfxx::map_t<uint16_t, pcibus_domain_registry_ptr>(kfxx::kernel_allocator());

	KM_RETURN_IF_FAILED(pcibus_scan_acpi_mcfg_table());

	return KM_RESULT_OK;
}

PBOS_USED PBOS_KMOD_API void pbos_module_deinit() {
	kxi_call_dtors();
}

PBOS_EXTERN_C_END
