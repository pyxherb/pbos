#ifndef _PBOS_KN_KM_EXEC_H_
#define _PBOS_KN_KM_EXEC_H_

#include <pbos/kf/rbtree.h>
#include <pbos/km/exec.hh>
#include "proc.hh"

PBOS_EXTERN_C_BEGIN

extern km_init_binldr_registry_t kn_builtin_binldrs[];

extern kfxx::rbtree_t<fs_fcb_t *> kn_registered_binprotos;

bool kn_binldr_reg_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
void kn_binldr_reg_nodefree(kf_rbtree_node_t *p);

void kn_load_init();
void kn_init_binldrs();

PBOS_EXTERN_C_END

#endif
