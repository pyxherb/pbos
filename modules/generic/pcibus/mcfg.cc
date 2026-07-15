#include "mcfg.h"
#include <pbos/acpi/misc.h>
#include <pbos/dm/device.h>
#include <pbos/kd/logger.h>
#include <string.h>
#include "device.h"

PBOS_EXTERN_C_BEGIN

km_result_t pcibus_scan_acpi_mcfg_table_and_create_domains() {
	if (!acpi_is_supported()) {
		kd_println(PCIROOT_COMPONENT_NAME, "ACPI is not available, skipping scanning ACPI MCFG table");
		return KM_RESULT_OK;
	}
	mm_context_t *context = mm_get_cur_context();
	void *mcfg_table_base = nullptr;
	for (size_t i = 0; i < acpi_rsdt_length(); ++i) {
		const acpi_sdt_header_t *header = acpi_rsdt_vaddr_at(i);

		if (!memcmp(&header->signature, "MCFG", sizeof(header->signature))) {
			kd_println(PCIROOT_COMPONENT_NAME, "Found ACPI MCFG header at %p", header);
			for (size_t j = sizeof(acpi_sdt_header_t) + 8; j < header->length; j += sizeof(pci_mcfg_entry_t)) {
				pci_mcfg_entry_t entry;

				// Note that the entry is not as aligned as the maximum aligned member,
				// so we have to make it aligned first to avoid unaligned access.
				memcpy(&entry, (pci_mcfg_entry_t *)(((char *)header) + j), sizeof(entry));

				kd_println(PCIROOT_COMPONENT_NAME, "Found PCI segment: %.4x", entry.pci_segment_group_num);

				if (pcibus_domain_tree.find(entry.pci_segment_group_num))
					continue;

				pcibus_domain_ptr registry = pcibus_domain_t::alloc();

				if (!registry)
					return KM_RESULT_NO_MEM;

				// Set the physical address.
				registry->ecam_pbase = reinterpret_cast<void *>(entry.ecam_base);

				void *ecam_vbase = mm_kvmalloc(mm_get_cur_context(), PCIBUS_ECAM_SIZE_PER_SEGMENT, 0, 0);

				if (!ecam_vbase)
					return KM_RESULT_NO_MEM;

				KM_RETURN_IF_FAILED(mm_mmap(mm_get_cur_context(), ecam_vbase, registry->ecam_pbase, PCIBUS_ECAM_SIZE_PER_SEGMENT, MM_PAGE_READ | MM_PAGE_WRITE, 0));

				// Add the registry to the segment group ID map.
				registry->segment_group_id = entry.pci_segment_group_num;

				if (!pcibus_segment_group_id_to_domain_map->insert(+entry.pci_segment_group_num, registry.get())) {
					return KM_RESULT_NO_MEM;
				}

				kfxx::scope_guard remove_from_segment_group_guard([registry]() noexcept {
					pcibus_segment_group_id_to_domain_map->remove(registry->segment_group_id);
				});

				// Allocate the domain ID and add it into the domain ID tree.
				if (!pcibus_alloc_domain_id_and_insert(registry.get()))
					return KM_RESULT_NO_SLOT;

				remove_from_segment_group_guard.release();

				kd_println(PCIROOT_COMPONENT_NAME, "Registered PCI segment -> domain registry: %.4x -> %.4x", entry.pci_segment_group_num, registry->rb_value);

				{
					char name[sizeof("0000")] = {};

					name[0] = (registry->rb_value & 0xff) + '0';
					name[1] = ((registry->rb_value >> 4) & 0xff) + '0';
					name[2] = ((registry->rb_value >> 8) & 0xff) + '0';
					name[3] = ((registry->rb_value >> 12) & 0xff) + '0';

					// dm_create_device(pcibus_bus_object, pcibus_bus_controller_device_class, /* TODO: Fill it. */, /* TODO: Fill it. */);

					kd_println(PCIROOT_COMPONENT_NAME, "Created device for PCI domain: %s", name);
				}
			}
			break;
		}
	}

	return KM_RESULT_OK;
}

PBOS_EXTERN_C_END
