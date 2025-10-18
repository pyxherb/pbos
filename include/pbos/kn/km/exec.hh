#ifndef _PBOS_KN_KM_EXEC_H_
#define _PBOS_KN_KM_EXEC_H_

#include "proc.hh"
#include <pbos/km/exec.h>
#include <pbos/kf/rbtree.h>

PBOS_EXTERN_C_BEGIN

extern km_binldr_t* kn_builtin_binldrs[];
extern kf_rbtree_t kn_registered_binldrs;

bool kn_binldr_reg_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
void kn_binldr_reg_nodefree(kf_rbtree_node_t *p);

void kn_load_init();
void kn_init_binldrs();

PBOS_EXTERN_C_END

#endif
