#include <pbos/kh/ds/blfb.h>

PBOS_EXTERN_C_BEGIN

extern ki_blfb_desc_t hali_blfb_descs[256];
extern size_t hali_blfb_desc_num;

void hali_push_blfb_desc(const ki_blfb_desc_t &desc);

PBOS_EXTERN_C_END
