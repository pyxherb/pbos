#ifndef _HAL_INIT_H_
#define _HAL_INIT_H_

#include <pbos/common.h>

PB_EXTERN_C_BEGIN

void hal_init();
void hal_call_ctors();

PB_EXTERN_C_END

#endif
