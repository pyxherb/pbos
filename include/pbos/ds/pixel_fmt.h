#ifndef _PBOS_DS_PIXEL_FMT_H_
#define _PBOS_DS_PIXEL_FMT_H_

#include <pbos/km/result.h>

enum {
	/// @brief Unknown pixel format.
	DS_PIXEL_FORMAT_UNKNOWN = 0,

	/// @brief RGB565, 16-bit unsigned integer.
	DS_PIXEL_FORMAT_R5G6B5_UINT,
	DS_PIXEL_FORMAT_B5G6R5_UINT,
	/// @brief RGB888, 24-bit unsigned integer.
	DS_PIXEL_FORMAT_R8G8B8_UINT,
	DS_PIXEL_FORMAT_B8G8R8_UINT,
};

typedef uint32_t ds_pixel_format_t;

#endif
