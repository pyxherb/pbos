#ifndef _PBOS_KM_EXEC_HH_
#define _PBOS_KM_EXEC_HH_

#include "exec.h"
#include <pbos/kfxx/rbtree.hh>
#include <pbos/kfxx/uuid.hh>

PBOS_EXTERN_C_BEGIN

typedef struct _kn_binldr_registry_t : public kfxx::rbtree_t<kf_uuid_t>::node_t {
	km_binldr_t binldr;
} kn_binldr_registry_t;

extern kfxx::rbtree_t<kf_uuid_t> kn_registered_binldrs;

PBOS_EXTERN_C_END

#endif
