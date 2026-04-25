#include <pbos/kn/acpi/rsdt.hh>

void *kn_acpi_rsdp_paddr = nullptr;
acpi_rsdp_t *kn_acpi_rsdp_vaddr = nullptr;
acpi_sdt_header_t *kn_acpi_rsdt_vaddr, **kn_mapped_acpi_rsdt_entries = nullptr;

bool kn_acpi_verify_checksum(const char *data, size_t size) {
	uint8_t sum = 0;
	for (size_t i = 0; i < size; ++i) {
		sum += ((uint8_t *)data)[i];
	}
	return !sum;
}
