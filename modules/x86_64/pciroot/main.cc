#include <pbos/dm/bus.h>
#include <pbos/kd/logger.h>
#include <pbos/pci/mcfg.h>
#include <string.h>
#include <pbos/kfxx/rbtree.hh>

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

struct pci_segment_map_entry_t : public kfxx::rbtree_t<uint16_t>::node_t {
	dm_device_t *device;
};

kfxx::rbtree_t<uint16_t> pci_segment_map;

km_result_t scan_acpi_mcfg_table() {
	mm_context_t *context = mm_get_cur_context();
	void *mcfg_table_base = nullptr;
	for(size_t i = 0 ; i < acpi_rsdt_length(); ++i) {
		acpi_sdt_header_t *header = acpi_rsdt_vaddr_at(i);

		if(!memcmp(&header->signature, "MCFG", sizeof(header->signature))) {
			for(size_t j = sizeof(acpi_sdt_header_t) + 8; j < header->length; ++j) {
				pci_mcfg_entry_t entry;

				memcpy(&entry, (pci_mcfg_entry_t*)(((char*)header) + j), sizeof(entry));
			}
			break;
		}
	}
}

PBOS_KMOD_API km_result_t module_init() {
	return KM_RESULT_OK;
}

PBOS_KMOD_API void module_deinit() {
}

PBOS_EXTERN_C_END
