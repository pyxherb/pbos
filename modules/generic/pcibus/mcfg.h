#ifndef _PCIROOT_MCFG_H_
#define _PCIROOT_MCFG_H_

#include "common.h"
#include <pbos/pci/mcfg.h>

PBOS_EXTERN_C_BEGIN

km_result_t pcibus_scan_acpi_mcfg_table_and_create_resources();

PBOS_EXTERN_C_END

#endif
