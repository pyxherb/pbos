#ifndef _PBOS_KI_DS_BLFB_H_
#define _PBOS_KI_DS_BLFB_H_

#include <pbos/ds/blfb.h>

PBOS_EXTERN_C_BEGIN

struct ki_blfb_desc_t {
	void *paddr, *mapped_vaddr;
	ds_pixel_format_t pixel_format;
	uint64_t width, height;
	size_t stride;
};

struct ki_blfb_pixel_format_case_t {
	struct {
		uint8_t shift;
		uint8_t size;
	} red;

	struct {
		uint8_t shift;
		uint8_t size;
	} green;

	struct {
		uint8_t shift;
		uint8_t size;
	} blue;

	ds_pixel_format_t format;
};

extern ki_blfb_pixel_format_case_t ki_blfb_pixel_format_cases[];
extern const size_t ki_blfb_pixel_format_case_num;

PBOS_EXTERN_C_END

#endif
