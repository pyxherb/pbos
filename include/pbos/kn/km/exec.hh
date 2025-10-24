#ifndef _PBOS_KN_KM_EXEC_H_
#define _PBOS_KN_KM_EXEC_H_

#include "proc.hh"
#include <pbos/km/exec.hh>
#include <pbos/kf/rbtree.h>

PBOS_EXTERN_C_BEGIN

extern km_init_binldr_registry_t kn_builtin_binldrs[];

bool kn_binldr_reg_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
void kn_binldr_reg_nodefree(kf_rbtree_node_t *p);

void kn_load_init();
void kn_init_binldrs();

PBOS_EXTERN_C_END

#endif
