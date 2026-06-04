#include "mcfg.h"
#include <pbos/kd/logger.h>
#include <string.h>

PBOS_EXTERN_C_BEGIN

km_result_t pciroot_scan_acpi_mcfg_table() {
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

				if (pciroot_domain_tree.find(entry.pci_segment_group_num))
					continue;

				pciroot_domain_registry_ptr registry = pciroot_domain_registry_t::alloc();

				if (!registry)
					return KM_RESULT_NO_MEM;

				// Add the registry to the segment group ID map.
				registry->segment_group_id = entry.pci_segment_group_num;

				if (!pciroot_segment_group_id_to_domain_map->insert(+entry.pci_segment_group_num, registry.get())) {
					return KM_RESULT_NO_MEM;
				}

				kfxx::scope_guard remove_from_segment_group_guard([registry]() noexcept {
					pciroot_segment_group_id_to_domain_map->remove(registry->segment_group_id);
				});

				// Allocate the domain ID and add it into the domain ID tree.
				if (!pciroot_alloc_domain_id_and_insert(registry.get()))
					return KM_RESULT_NO_SLOT;

				remove_from_segment_group_guard.release();

				kd_println(PCIROOT_COMPONENT_NAME, "Registered PCI segment -> domain registry: %.4x -> %.4x", entry.pci_segment_group_num, registry->rb_value);

				// TODO: Implement it.
				char name[sizeof("0000")] = {};

				name[0] = (registry->rb_value & 0xff) + '0';
				name[1] = ((registry->rb_value >> 4) & 0xff) + '0';
				name[2] = ((registry->rb_value >> 8) & 0xff) + '0';
				name[3] = ((registry->rb_value >> 12) & 0xff) + '0';

				kd_println(PCIROOT_COMPONENT_NAME, "Created directory for PCI segment: %s", name);
			}
			break;
		}
	}

	return KM_RESULT_OK;
}

PBOS_EXTERN_C_END
