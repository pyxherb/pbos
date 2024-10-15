#ifndef _HAL_MISC_H_
#define _HAL_MISC_H_

#include <pbos/common.h>

enum {
	HAL_CAP_LOGGER = 0,
	HAL_CAP_DMA
};

bool hal_iscapable(uint16_t cap);

#endif
