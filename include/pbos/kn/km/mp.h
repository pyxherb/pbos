#ifndef _PBOS_KN_KM_MP_H_
#define _PBOS_KN_KM_MP_H_

#include <pbos/km/proc.h>
#include <pbos/kfxx/rbtree.hh>
#include <functional>

PBOS_EXTERN_C_BEGIN

typedef struct _mp_cpu_t : public kfxx::rbtree_t<ps_cpu_id_t>::node_t {
} mp_cpu_t;

extern uint32_t mp_num_total_cpu;
extern kfxx::rbtree_t<mp_cpu_t *> mp_total_cpu_set;

void mp_init();

///
/// @brief Initialize MP for current CPU.
///
void mp_main_cpu_init();

PBOS_EXTERN_C_END

#endif
