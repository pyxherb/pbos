#ifndef _OICOS_PM_ACPI_H_
#define _OICOS_PM_ACPI_H_

#include <oicos/common.h>

typedef struct _acpi_rsdp {
	uint8_t signature[8];
	uint8_t checksum;
	uint8_t oemid[6];
	uint8_t version;
	void *rsdt;
} acpi_rsdp;

#endif
