#ifndef _PBOS_KN_ACPI_RSDT_HH_
#define _PBOS_KN_ACPI_RSDT_HH_

#include <pbos/acpi/rsdt.h>
#include <pbos/km/assert.h>

PBOS_NODISCARD bool kn_acpi_verify_checksum(const char *data, size_t size);
PBOS_FORCEINLINE uint32_t kn_acpi_rsdt_length(acpi_sdt_header_t *base) {
	return (base->length - sizeof(acpi_sdt_header_t)) / sizeof(uint32_t);
}
PBOS_FORCEINLINE uint32_t kn_acpi_rsdt_at(acpi_sdt_header_t *base, uint32_t index) {
	kd_dbgcheck(
		index < kn_acpi_rsdt_length(base),
		"ACPI RSDT out of range");
	return ((uint32_t *)(&base[1]))[index];
}

extern void *kn_acpi_rsdp_paddr;
extern acpi_rsdp_t *kn_acpi_rsdp_vaddr;

#endif
