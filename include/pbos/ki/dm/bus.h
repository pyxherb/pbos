#ifndef _PBOS_KI_DM_BUS_H_
#define _PBOS_KI_DM_BUS_H_

#include <pbos/dm/bus.h>
#include <pbos/kfxx/rbtree.hh>

typedef struct _dm_bus_registry_t : kfxx::rbtree_t<kf_uuid_t>::node_t {
} dm_bus_registry_t;

typedef struct _dm_bus_t {

} dm_bus_t;

#endif
