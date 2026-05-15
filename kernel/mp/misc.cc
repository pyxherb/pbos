#include <pbos/ki/mp/misc.hh>

PBOS_EXTERN_C_BEGIN

uint32_t mp_num_total_cpu;
kfxx::RBTree<mp_cpu_t *> mp_total_cpu_set;

PBOS_EXTERN_C_END
