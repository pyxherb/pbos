#include <pbos/kh/ds/blfb.h>

ki_blfb_pixel_format_case_t ki_blfb_pixel_format_cases[] = {
	{ .red = { 0, 5 }, .green = { 5, 6 }, .blue = { 11, 5 }, .format = DS_PIXEL_FORMAT_R5G6B5_UINT },
	{ .red = { 11, 5 }, .green = { 5, 6 }, .blue = { 0, 5 }, .format = DS_PIXEL_FORMAT_B5G6R5_UINT },
	{ .red = { 0, 8 }, .green = { 8, 8 }, .blue = { 16, 8 }, .format = DS_PIXEL_FORMAT_R8G8B8_UINT },
	{ .red = { 16, 8 }, .green = { 8, 8 }, .blue = { 0, 8 }, .format = DS_PIXEL_FORMAT_B8G8R8_UINT }
};

const size_t ki_blfb_pixel_format_case_num = PBOS_ARRAYSIZE(ki_blfb_pixel_format_cases);

PBOS_API void ds_offload_boot_fb() {

}

PBOS_API size_t ds_get_blfb_num() {
	return kh_get_blfb_num();
}
