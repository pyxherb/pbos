#include <pbos/kf/atomic.h>
#include <pbos/ki/dm/device.h>
#include <string.h>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/km/symbol.hh>

ki_dm_device_allocator_t::ki_dm_device_allocator_t() {
}

ki_dm_device_allocator_t::~ki_dm_device_allocator_t() {
}

size_t ki_dm_device_allocator_t::inc_ref() noexcept {
	return 0;
}

size_t ki_dm_device_allocator_t::dec_ref() noexcept {
	return 0;
}

void *ki_dm_device_allocator_t::alloc(size_t size, size_t alignment) noexcept {
	return mm_kalloc(size, alignment);
}

void *ki_dm_device_allocator_t::realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	return mm_krealloc(ptr, new_size, new_alignment);
}

void *ki_dm_device_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size) noexcept {
	return mm_krealloc_in_place(ptr, new_size);
}

void ki_dm_device_allocator_t::release(void *ptr, size_t size, size_t alignment) noexcept {
	mm_kfree(ptr);
}

int ki_dm_device_allocator_identity;

void *ki_dm_device_allocator_t::type_identity() const noexcept {
	return &ki_dm_device_allocator_identity;
}

ki_dm_device_allocator_t ki_dm_device_allocator;

_dm_device_t::_dm_device_t() {
}

km_result_t ki_dm_alloc_device(dm_bus_t *bus, dm_device_class_t *device_class, dm_device_ops_t *ops, dm_device_t **device_out) {
	dm_device_t *device = (dm_device_t *)kfxx::alloc_and_construct<dm_device_t>(&ki_dm_device_allocator);

	if (!device)
		return KM_RESULT_NO_MEM;

	device->ops.destroy = nullptr;

	kfxx::scope_guard release_device_guard([device]() noexcept {
		ki_dm_destroy_device(device);
	});

	if (!bus->owned_devices.insert(+device))
		return KM_RESULT_NO_MEM;
	kfxx::scope_guard remove_device_from_bus_guard([bus, device]() noexcept {
		bus->owned_devices.remove(device);
	});

	if (!device_class->owned_devices.insert(+device))
		return KM_RESULT_NO_MEM;
	kfxx::scope_guard remove_device_from_class_guard([device_class, device]() noexcept {
		device_class->owned_devices.remove(device);
	});

	memcpy(&device->ops, ops, sizeof(*ops));

	release_device_guard.release();
	remove_device_from_bus_guard.release();
	remove_device_from_class_guard.release();

	*device_out = device;

	return KM_RESULT_OK;
}

void ki_dm_destroy_device(dm_device_t *device) {
	kd_assert(!device->parent_device);

	if (device->ops.destroy)
		device->ops.destroy(device);

	kfxx::destroy_and_release<dm_device_t>(&ki_dm_device_allocator, device);
}

PBOS_API void dm_ref_device(dm_device_t *device) {
	kf_atomic_inc_size(&device->ref_count);
}

PBOS_API void dm_unref_device(dm_device_t *device) {
	if (!kf_atomic_dec_size(&device->ref_count))
		ki_dm_destroy_device(device);
}

PBOS_API km_result_t dm_link_device(dm_device_t *parent, dm_device_t *device) {
	ps::rec_mutex_guard gp(parent->device_mutex.c_mutex()),
		gd(device->device_mutex.c_mutex());

	// The device must be already registered to a bus.
	if(!device->bus)
		return KM_RESULT_INVALID_ARGS;

	KM_RETURN_IF_FAILED(parent->ops.prelink(parent, device));

	if ((device->parent_device) ||
		(device->prev_same_level) ||
		(device->next_same_level))
		return KM_RESULT_INVALID_ARGS;

	if (parent->child_devices)
		parent->child_devices->prev_same_level = device;
	device->next_same_level = parent->child_devices;

	dm_ref_device(device);

	parent->child_devices = parent;

	return KM_RESULT_OK;
}

PBOS_API void dm_unlink_device(dm_device_t *device) {
	ps::rec_mutex_guard gp(device->parent_device->device_mutex.c_mutex()),
		gd(device->device_mutex.c_mutex());

	if (!device->parent_device)
		return;

	device->parent_device->ops.unlink_cleanup(device->parent_device, device);

	if (device->prev_same_level == device->parent_device->child_devices)
		device->parent_device->child_devices = device->next_same_level;
	if (device->prev_same_level) {
		device->prev_same_level->next_same_level = device->next_same_level;
	}
	if (device->next_same_level) {
		device->next_same_level->prev_same_level = device->prev_same_level;
	}

	dm_unref_device(device);
}

PBOS_API void dm_set_device_exdata(dm_device_t *device, void *exdata) {
	device->exdata = exdata;
}

PBOS_API void *dm_get_device_exdata(dm_device_t *device) {
	return device->exdata;
}
