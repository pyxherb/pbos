#ifndef _PBOS_KN_ACPI_RSDT_HH_
#define _PBOS_KN_ACPI_RSDT_HH_

#include <pbos/acpi/rsdt.h>
#include <pbos/km/assert.h>

extern void *kn_acpi_rsdp_paddr;
extern acpi_rsdp_t *kn_acpi_rsdp_vaddr;
extern acpi_sdt_header_t *kn_acpi_rsdt_vaddr, **kn_mapped_acpi_rsdt_entries;

PBOS_NODISCARD bool kn_acpi_verify_checksum(const char *data, size_t size);
uint32_t kn_acpi_rsdt_length();
void *kn_acpi_rsdt_paddr_at(size_t index);
acpi_sdt_header_t *kn_acpi_rsdt_vaddr_at(size_t index);

#endif
