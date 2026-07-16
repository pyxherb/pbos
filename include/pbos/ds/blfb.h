#ifndef _PBOS_DS_BLFB_H_
#define _PBOS_DS_BLFB_H_

#include "pixel_fmt.h"

PBOS_EXTERN_C_BEGIN

typedef uint32_t ds_blfb_display_id_t;

PBOS_API void ds_offload_blfb();
PBOS_API size_t ds_get_blfb_num();

PBOS_EXTERN_C_END

#endif
