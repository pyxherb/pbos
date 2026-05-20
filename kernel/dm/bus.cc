#include <pbos/kf/atomic.h>
#include <pbos/ki/dm/bus.h>
#include <pbos/ki/dm/device.h>
#include <string.h>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/km/symbol.hh>

ki_dm_bus_allocator_t::ki_dm_bus_allocator_t() {
}

ki_dm_bus_allocator_t::~ki_dm_bus_allocator_t() {
}

size_t ki_dm_bus_allocator_t::inc_ref() noexcept {
	return 0;
}

size_t ki_dm_bus_allocator_t::dec_ref() noexcept {
	return 0;
}

void *ki_dm_bus_allocator_t::alloc(size_t size, size_t alignment) noexcept {
	return mm_kalloc(size, alignment);
}

void *ki_dm_bus_allocator_t::realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	return mm_krealloc(ptr, new_size, new_alignment);
}

void *ki_dm_bus_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	return mm_krealloc_in_place(ptr, new_size, new_alignment);
}

void ki_dm_bus_allocator_t::release(void *ptr, size_t size, size_t alignment) noexcept {
	mm_kfree(ptr);
}

int ki_dm_bus_allocator_identity;

void *ki_dm_bus_allocator_t::type_identity() const noexcept {
	return &ki_dm_bus_allocator_identity;
}

ki_dm_bus_allocator_t ki_dm_bus_allocator;

_dm_bus_t::_dm_bus_t() : owned_devices(&ki_dm_bus_allocator) {}

PBOS_KERNEL_PUBLIC void dm_set_bus_exdata(dm_bus_t *bus, void *exdata) {
	bus->exdata = exdata;
}

PBOS_KERNEL_PUBLIC void *dm_get_bus_exdata(dm_bus_t *bus) {
	return bus->exdata;
}

kfxx::HashMap<kfxx::StringView, dm_bus_t *> ki_dm_registered_buses(&ki_dm_bus_allocator);
ps::Mutex ki_dm_registered_buses_mutex;

PBOS_KERNEL_PUBLIC km_result_t dm_register_bus(const char *name, size_t name_len, const dm_bus_ops_t *ops) {
	dm_bus_t *bus;

	KM_RETURN_IF_FAILED(ki_dm_alloc_bus(name, name_len, ops, &bus));

	kfxx::ScopeGuard release_bus_guard([bus]() noexcept {
		ki_dm_destroy_bus(bus);
	});

	ps::MutexGuard g(ki_dm_registered_buses_mutex.c_mutex());

	if (!ki_dm_registered_buses.shrink_buckets())
		return KM_RESULT_NO_MEM;

	if (!ki_dm_registered_buses.insert(kfxx::StringView(bus->name, bus->name_len), +bus))
		return KM_RESULT_NO_MEM;

	release_bus_guard.release();

	return KM_RESULT_OK;
}

PBOS_KERNEL_PUBLIC void dm_unregister_bus(dm_bus_t *bus) {
	ps::MutexGuard g(ki_dm_registered_buses_mutex.c_mutex());

	ki_dm_registered_buses.remove(bus->name);
	ki_dm_destroy_bus(bus);
}

PBOS_KERNEL_PUBLIC km_result_t dm_register_device_to_bus(dm_bus_t *bus, dm_device_t *device) {
	ps::RecMutexGuard bg(bus->bus_mutex.c_mutex());
	ps::RecMutexGuard dg(device->device_mutex.c_mutex());
	if(!bus->owned_devices.insert(+device))
		return KM_RESULT_NO_MEM;

	kfxx::ScopeGuard remove_device_from_bus_guard([bus, device]() noexcept {
		bus->owned_devices.remove(device);
	});

	KM_RETURN_IF_FAILED(bus->ops.register_device(bus, device));

	remove_device_from_bus_guard.release();

	return KM_RESULT_OK;
}

PBOS_KERNEL_PUBLIC void dm_unregister_device_from_bus(dm_device_t *device) {
	ps::RecMutexGuard bg(device->bus->bus_mutex.c_mutex());
	ps::RecMutexGuard dg(device->device_mutex.c_mutex());
	device->bus->ops.unregister_device(device->bus, device);
	device->bus->owned_devices.remove(device);
}

km_result_t ki_dm_alloc_bus(const char *name, size_t name_len, const dm_bus_ops_t *ops, dm_bus_t **bus_out) {
	dm_bus_t *bus = kfxx::alloc_and_construct<dm_bus_t>(&ki_dm_bus_allocator);

	if (!bus)
		return KM_RESULT_NO_MEM;

	kfxx::ScopeGuard release_bus_guard([bus]() noexcept {
		ki_dm_destroy_bus(bus);
	});

	if (!(bus->name = (char *)mm_kalloc(name_len, alignof(char))))
		return KM_RESULT_NO_MEM;
	bus->name_len = name_len;
	memcpy(bus->name, name, name_len);

	memcpy(&bus->ops, ops, sizeof(*ops));

	release_bus_guard.release();

	return KM_RESULT_OK;
}

void ki_dm_destroy_bus(dm_bus_t *bus) {
	bus->ops.destroy_bus(bus);
	kfxx::destroy_and_release<dm_bus_t>(&ki_dm_bus_allocator, bus);
}
