#ifndef _PBOS_KI_MP_INIT_H_
#define _PBOS_KI_MP_INIT_H_

#include "misc.hh"

PBOS_EXTERN_C_BEGIN

void mp_init();

/// @brief Allocate MP resources.
/// @note This function should be called after the topology detection.
void ki_mp_alloc_resources();

///
/// @brief Initialize MP for current CPU.
///
void mp_main_cpu_init();

PBOS_EXTERN_C_END

#endif
