#ifndef _PBOS_KH_MM_INIT_HH_
#define _PBOS_KH_MM_INIT_HH_

#include "misc.hh"

PBOS_EXTERN_C_BEGIN

void ki_mm_init_global_allocator();

PBOS_NO_ASAN void kh_mm_init();

PBOS_EXTERN_C_END

#endif
