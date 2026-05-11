#ifndef _PBOS_KI_DM_BUS_H_
#define _PBOS_KI_DM_BUS_H_

#include <pbos/dm/bus.h>
#include <pbos/kfxx/map.hh>

typedef struct _dm_bus_type_t {
	char *name;
	size_t name_len;
} dm_bus_type_t;

typedef struct _dm_bus_t {
	dm_bus_t *parent_bus;
	dm_bus_type_t *bus_type;
	kfxx::set_t<dm_device_t *> owned_devices;
	void *exdata;
} dm_bus_t;

#endif
