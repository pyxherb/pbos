#ifndef _PBKXRT_INIT_H_
#define _PBKXRT_INIT_H_

#include <pbos/km/result.h>

PBOS_EXTERN_C_BEGIN

PBOS_USED PBOS_KMOD_API km_result_t module_init();
PBOS_USED PBOS_KMOD_API void module_deinit();

void kxi_call_ctors();
void kxi_call_dtors();

PBOS_EXTERN_C_END

#endif
