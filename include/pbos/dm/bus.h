#ifndef _PBOS_DM_BUS_H_
#define _PBOS_DM_BUS_H_

#include <pbos/fs/file.h>
#include <pbos/se/user.h>
#include <pbos/ps/proc.h>

PBOS_EXTERN_C_BEGIN

typedef struct _dm_bus_type_t dm_bus_type_t;
typedef struct _dm_bus_t dm_bus_t;

typedef struct _dm_device_t dm_device_t;

typedef struct _dm_bus_ops_t {
	void (*destroy_bus)(dm_bus_t *bus);
	void (*destroy_device)(dm_device_t *device);
} dm_bus_ops_t;

PBOS_EXTERN_C_END

#endif
