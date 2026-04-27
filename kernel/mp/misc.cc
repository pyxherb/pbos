#include <pbos/kn/km/mp.h>

PBOS_EXTERN_C_BEGIN

uint32_t mp_num_total_cpu;
kfxx::rbtree_t<mp_cpu_t *> mp_total_cpu_set;

PBOS_EXTERN_C_END
