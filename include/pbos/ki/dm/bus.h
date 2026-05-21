#ifndef _PBOS_KI_DM_BUS_H_
#define _PBOS_KI_DM_BUS_H_

#include <pbos/dm/bus.h>
#include <pbos/kfxx/map.hh>
#include <pbos/ps/mutex.hh>
#include <pbos/kfxx/hashmap.hh>
#include <pbos/kfxx/string_view.hh>

class ki_dm_bus_allocator_t : public kfxx::allocator_t {
public:
	PBOS_PRIVATE ki_dm_bus_allocator_t();
	PBOS_PRIVATE virtual ~ki_dm_bus_allocator_t();

	PBOS_PRIVATE virtual size_t inc_ref() noexcept override;
	PBOS_PRIVATE virtual size_t dec_ref() noexcept override;

	PBOS_PRIVATE virtual void *alloc(size_t size, size_t alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
	PBOS_PRIVATE virtual void release(void *ptr, size_t size, size_t alignment) noexcept override;

	PBOS_PRIVATE virtual void *type_identity() const noexcept override;
};

typedef struct _dm_bus_t {
	char *name;
	size_t name_len;

	dm_bus_ops_t ops;
	kfxx::set_t<dm_device_t *> owned_devices;
	ps::rec_mutex_t bus_mutex;
	void *exdata;

	_dm_bus_t();
} dm_bus_t;

extern kfxx::hashmap_t<kfxx::string_view, dm_bus_t *> ki_dm_registered_buses;

km_result_t ki_dm_alloc_bus(const char *name, size_t name_len, const dm_bus_ops_t *ops, dm_bus_t **bus_out);
void ki_dm_destroy_bus(dm_bus_t *bus);

#endif
