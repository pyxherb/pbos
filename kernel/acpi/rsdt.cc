#include <pbos/ki/acpi/rsdt.hh>

void *ki_acpi_rsdp_paddr = nullptr;
acpi_rsdp_t *ki_acpi_rsdp_vaddr = nullptr;
acpi_sdt_header_t *ki_acpi_rsdt_vaddr = nullptr, **ki_mapped_acpi_rsdt_entries = nullptr;

bool ki_acpi_verify_checksum(const char *data, size_t size) {
	uint8_t sum = 0;
	for (size_t i = 0; i < size; ++i) {
		sum += ((uint8_t *)data)[i];
	}
	return !sum;
}

uint32_t ki_acpi_rsdt_length() {
	kd_assert(ki_acpi_rsdt_vaddr);
	return (ki_acpi_rsdt_vaddr->length - sizeof(acpi_sdt_header_t)) / sizeof(uint32_t);
}

void *ki_acpi_rsdt_paddr_at(size_t index) {
	kd_assert(ki_acpi_rsdt_vaddr);
	kd_dbgcheck(
		index < ki_acpi_rsdt_length(),
		"ACPI RSDT out of range");
	switch (ki_acpi_rsdp_vaddr->revision) {
		case 0:
			kd_dbgcheck(index <= UINT32_MAX, "Invalid index for RSDT");
			return (void*)(size_t)((uint32_t *)(&ki_acpi_rsdt_vaddr[1]))[index];
		case 2:
			return (void*)(size_t)((uint64_t *)(&ki_acpi_rsdt_vaddr[1]))[index];
	}
	PBOS_UNREACHABLE();
}

acpi_sdt_header_t *ki_acpi_rsdt_vaddr_at(size_t index) {
	return ki_mapped_acpi_rsdt_entries[index];
}
