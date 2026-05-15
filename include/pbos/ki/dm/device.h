#ifndef _PBOS_KI_DM_DEVICE_H_
#define _PBOS_KI_DM_DEVICE_H_

#include "bus.h"
#include <pbos/dm/device.h>

class ki_dm_device_allocator_t : public kfxx::Alloc {
public:
	PBOS_PRIVATE ki_dm_device_allocator_t();
	PBOS_PRIVATE virtual ~ki_dm_device_allocator_t();

	PBOS_PRIVATE virtual size_t inc_ref() noexcept override;
	PBOS_PRIVATE virtual size_t dec_ref() noexcept override;

	PBOS_PRIVATE virtual void *alloc(size_t size, size_t alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
	PBOS_PRIVATE virtual void release(void *ptr, size_t size, size_t alignment) noexcept override;

	PBOS_PRIVATE virtual void *type_identity() const noexcept override;
};

typedef struct _dm_device_class_t {
	char *name;
	size_t name_len;

	kfxx::Set<dm_device_t *> owned_devices;

	ps::Mutex bus_mutex;
} dm_device_class_t;

typedef struct _dm_device_t {
	size_t ref_count = 0;

	dm_device_t *parent_device;

	dm_device_t *prev_same_level = nullptr, *next_same_level = nullptr;
	dm_device_t *child_devices = nullptr;

	dm_bus_t *bus = nullptr;
	dm_device_class_t *device_class = nullptr;

	dm_device_ops_t ops;

	ps::RecMutex device_mutex;

	void *exdata = nullptr;

	_dm_device_t();
} dm_device_t;

km_result_t ki_dm_alloc_device(dm_bus_t *bus, dm_device_class_t *device_class, dm_device_ops_t *ops, dm_device_t **device_out);
void ki_dm_destroy_device(dm_device_t *device);

#endif
