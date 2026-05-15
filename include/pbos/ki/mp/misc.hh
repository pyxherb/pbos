#ifndef _PBOS_KI_MP_MISC_H_
#define _PBOS_KI_MP_MISC_H_

#include <pbos/ps/proc.h>
#include <pbos/kfxx/rbtree.hh>
#include <functional>

PBOS_EXTERN_C_BEGIN

#define MP_CPU_FLAGS_ENABLED 0x00000001

typedef uint32_t mp_cpu_flags_t;

typedef struct _mp_cpu_t : public kfxx::RBTree<ps_cpu_id_t>::node_t {
	mp_cpu_flags_t flags;
} mp_cpu_t;

extern uint32_t mp_num_total_cpu;
extern kfxx::RBTree<mp_cpu_t *> mp_total_cpu_set;

PBOS_EXTERN_C_END

#endif
