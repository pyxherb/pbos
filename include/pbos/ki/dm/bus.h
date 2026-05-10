#ifndef _PBOS_KI_DM_BUS_H_
#define _PBOS_KI_DM_BUS_H_

#include <pbos/dm/bus.h>
#include <pbos/kfxx/rbtree.hh>

typedef struct _dm_bus_type_t {
	char *name;
	size_t name_len;
} dm_bus_type_t;

typedef struct _dm_bus_t {
	dm_bus_t *parent_bus;
	dm_bus_type_t *bus_type;
	void *exdata;
} dm_bus_t;

typedef struct _dm_device_t {
	dm_device_t *parent_device;
	dm_bus_t *bus;
	kf_uuid_t device_class;
	void *exdata;
} dm_device_t;

#endif
