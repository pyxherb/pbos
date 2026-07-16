#include "blfb.hh"
#include <pbos/kd/assert.h>

PBOS_EXTERN_C_BEGIN

ki_blfb_desc_t hali_blfb_descs[256];
size_t hali_blfb_desc_num = 0;

size_t kh_get_blfb_num() {
	return hali_blfb_desc_num;
}

void hali_push_blfb_desc(const ki_blfb_desc_t &desc) {
	kd_assert(hali_blfb_desc_num < PBOS_ARRAYSIZE(hali_blfb_descs));

	hali_blfb_descs[hali_blfb_desc_num] = desc;
	++hali_blfb_desc_num;
}

PBOS_EXTERN_C_END
