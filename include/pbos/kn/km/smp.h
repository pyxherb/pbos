#ifndef _PBOS_KN_KM_SMP_H_
#define _PBOS_KN_KM_SMP_H_

#include <pbos/km/proc.h>

PBOS_EXTERN_C_BEGIN

void smp_init();

///
/// @brief Initialize SMP for current EU.
///
void smp_main_eu_init();

PBOS_EXTERN_C_END

#endif
