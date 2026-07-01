#ifndef _PBOS_KI_ACPI_MISC_HH_
#define _PBOS_KI_ACPI_MISC_HH_

#include <pbos/acpi/misc.h>
#include <pbos/ki/acpi/rsdt.hh>

bool kh_acpi_is_supported();
bool kh_acpi_is_required();
void kh_acpi_init();

#endif
