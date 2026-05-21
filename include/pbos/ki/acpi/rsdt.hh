#ifndef _PBOS_KI_ACPI_RSDT_HH_
#define _PBOS_KI_ACPI_RSDT_HH_

#include <pbos/acpi/rsdt.h>
#include <pbos/kd/assert.h>

extern void *ki_acpi_rsdp_paddr;
extern acpi_rsdp_t *ki_acpi_rsdp_vaddr;
extern acpi_sdt_header_t *ki_acpi_rsdt_vaddr, **ki_mapped_acpi_rsdt_entries;

#endif
