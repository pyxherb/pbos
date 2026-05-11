#ifndef _PBOS_KI_DM_DEVICE_H_
#define _PBOS_KI_DM_DEVICE_H_

#include "bus.h"
#include <pbos/dm/device.h>

typedef struct _dm_device_class_t {
	char *name;
	size_t name_len;
} dm_device_class_t;

typedef struct _dm_device_t {
	dm_device_t *parent_device;

	dm_device_t *prev_same_level = nullptr, *next_same_level = nullptr;
	dm_device_t *child_devices = nullptr;

	dm_bus_t *bus = nullptr;
	dm_device_class_t *device_class = nullptr;

	dm_device_ops_t ops;

	void *exdata = nullptr;

	_dm_device_t();
} dm_device_t;

#endif
