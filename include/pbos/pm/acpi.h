#ifndef _PBOS_PM_ACPI_H_
#define _PBOS_PM_ACPI_H_

#include <pbos/common.h>

typedef struct _acpi_rsdp {
	uint8_t signature[8];
	uint8_t checksum;
	uint8_t oemid[6];
	uint8_t version;
	void *rsdt;
} acpi_rsdp;

#endif
