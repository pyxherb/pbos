#ifndef _PBOS_DM_BUS_H_
#define _PBOS_DM_BUS_H_

#include <pbos/fs/file.h>
#include <pbos/se/user.h>
#include <pbos/ps/proc.h>

PBOS_EXTERN_C_BEGIN

typedef struct _dm_bus_t dm_bus_t;

typedef struct _dm_device_t dm_device_t;

typedef struct _dm_bus_ops_t {
	void (*destroy_bus)(dm_bus_t *bus);
	km_result_t (*register_device)(dm_bus_t *bus, dm_device_t *device);
	void (*unregister_device)(dm_bus_t *bus, dm_device_t *device);
} dm_bus_ops_t;

PBOS_API void dm_set_bus_exdata(dm_bus_t *bus, void *exdata);
PBOS_API void *dm_get_bus_exdata(dm_bus_t *bus);

PBOS_API km_result_t dm_register_bus(const char *name, size_t name_len, const dm_bus_ops_t *ops);
PBOS_API void dm_unregister_bus(dm_bus_t *bus);

PBOS_API km_result_t dm_register_device_to_bus(dm_bus_t *bus, dm_device_t *device);
PBOS_API void dm_unregister_device_from_bus(dm_device_t *device);

PBOS_EXTERN_C_END

#endif
