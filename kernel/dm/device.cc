#include <pbos/kd/logger.h>
#include <pbos/kf/atomic.h>
#include <pbos/ki/dm/device.h>
#include <string.h>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/fs/devio.hh>
#include <pbos/ki/km/symbol.hh>

kfxx::rbtree_t<kf_uuid_t> ki_registered_device_classes;

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

_dm_device_class_t::_dm_device_class_t(kfxx::allocator_t *allocator) : owned_devices(allocator) {
}

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

km_result_t dm_register_device_class(const kf_uuid_t *uuid, dm_device_class_t **device_class_out) {
	// TODO: Use more proper allocators.
	dm_device_class_t *dev_cls = kfxx::alloc_and_construct<dm_device_class_t>(kfxx::kernel_allocator(), kfxx::kernel_allocator());
	if (!dev_cls)
		return KM_RESULT_NO_MEM;

	kfxx::scope_guard delete_dev_cls_guard([dev_cls]() noexcept {
		kfxx::destroy_and_release<dm_device_class_t>(kfxx::kernel_allocator(), dev_cls);
	});

	if (!ki_registered_device_classes.insert(dev_cls))
		return KM_RESULT_EXISTED;

	kd_println(
		__func__,
		"Registered device class: %.08x-%.04hx-%.04hx-%.04hx-%.08hx%.04hx",
		uuid->a,
		uuid->b,
		uuid->c,
		uuid->d,
		uuid->e1, uuid->e2);

	*device_class_out = dev_cls;

	return KM_RESULT_OK;
}

dm_device_class_t *dm_query_device_class(const kf_uuid_t *uuid) {
	if (auto node = ki_registered_device_classes.find(*uuid); node)
		return static_cast<dm_device_class_t *>(node);
	return nullptr;
}

void dm_unregister_device_class(dm_device_class_t *device_class) {

	kd_println(
		__func__,
		"Registered device class: %.08x-%.04hx-%.04hx-%.04hx-%.08hx%.04hx",
		device_class->rb_value.a,
		device_class->rb_value.b,
		device_class->rb_value.c,
		device_class->rb_value.d,
		device_class->rb_value.e1, device_class->rb_value.e2);
	ki_registered_device_classes.remove(device_class);
}

PBOS_API void dm_ref_device(dm_device_t *device) {
	kf_atomic_inc_size(&device->ref_count);
}

PBOS_API void dm_unref_device(dm_device_t *device) {
	if (!kf_atomic_dec_size(&device->ref_count))
		ki_dm_destroy_device(device);
}

PBOS_API km_result_t dm_link_device(dm_device_t *parent, dm_device_t *device) {
	ps::rec_mutex_guard gp(parent->device_mutex),
		gd(device->device_mutex);

	// The device must be already registered to a bus.
	if (!device->bus)
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
	ps::rec_mutex_guard gp(device->parent_device->device_mutex),
		gd(device->device_mutex);

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

PBOS_API fs_fnode_t *dm_get_devio_root_dir() {
	return ki_devio_root_dir.get();
}

PBOS_API km_result_t dm_create_devio_file(dm_device_t *device, fs_fnode_t *parent, const char *filename, size_t filename_len, fs_fnode_t **fnode_out) {
	if (fs_filesys_of_fnode(parent) != ki_devio_filesys)
		return KM_RESULT_INVALID_ARGS;

	if (fs_get_fnode_type(parent) != FS_FNODE_TYPE_DIR)
		return KM_RESULT_INVALID_ARGS;

	fs::fnode_write_lock_guard g(parent);

	fs::fnode_ptr fnode;

	KM_RETURN_IF_FAILED(fs_alloc_file_fnode(ki_devio_filesys, fnode.get_addr_without_release()));

	ki_devio_file_exdata_t *exdata = (ki_devio_file_exdata_t *)mm_kalloc(sizeof(ki_devio_file_exdata_t), alignof(ki_devio_file_exdata_t));

	if (!exdata)
		return KM_RESULT_NO_MEM;

	fs_set_fnode_exdata(fnode.get(), exdata);

	// TODO: Increase ref count of the device.

	exdata->device = device;

	KM_RETURN_IF_FAILED(fs_rename_fnode(fnode.get(), filename, filename_len));

	KM_RETURN_IF_FAILED(fs_link_subnode(parent, fnode.get()));

	{
		ki_devio_dir_exdata_t *parent_exdata = (ki_devio_dir_exdata_t *)fs_get_fnode_exdata(parent);
		((ki_devio_dir_exdata_t *)fs_get_fnode_exdata(parent_exdata->first_child))->prev = fnode.get();
		exdata->next = parent_exdata->first_child;
		parent_exdata->first_child = fnode.get();
	}

	*fnode_out = fnode.release();

	return KM_RESULT_OK;
}

PBOS_API km_result_t dm_create_devio_dir(fs_fnode_t *parent, const char *filename, size_t filename_len, fs_fnode_t **fnode_out) {
	if (fs_filesys_of_fnode(parent) != ki_devio_filesys)
		return KM_RESULT_INVALID_ARGS;

	if (fs_get_fnode_type(parent) != FS_FNODE_TYPE_DIR)
		return KM_RESULT_INVALID_ARGS;

	fs::fnode_write_lock_guard g(parent);

	fs::fnode_ptr fnode;

	KM_RETURN_IF_FAILED(fs_alloc_dir_fnode(ki_devio_filesys, fnode.get_addr_without_release()));

	ki_devio_dir_exdata_t *exdata = (ki_devio_dir_exdata_t *)mm_kalloc(sizeof(ki_devio_dir_exdata_t), alignof(ki_devio_dir_exdata_t));

	if (!exdata)
		return KM_RESULT_NO_MEM;

	fs_set_fnode_exdata(fnode.get(), exdata);

	KM_RETURN_IF_FAILED(fs_rename_fnode(fnode.get(), filename, filename_len));

	KM_RETURN_IF_FAILED(fs_link_subnode(parent, fnode.get()));

	{
		ki_devio_dir_exdata_t *parent_exdata = (ki_devio_dir_exdata_t *)fs_get_fnode_exdata(parent);
		if (parent_exdata->first_child)
			((ki_devio_dir_exdata_t *)fs_get_fnode_exdata(parent_exdata->first_child))->prev = fnode.get();
		exdata->next = parent_exdata->first_child;
		parent_exdata->first_child = fnode.get();
	}

	*fnode_out = fnode.release();

	return KM_RESULT_OK;
}

PBOS_API km_result_t dm_offload_devio_fnode(fs_fnode_t *fnode) {
	if (fs_filesys_of_fnode(fnode) != ki_devio_filesys)
		return KM_RESULT_INVALID_ARGS;

	fs_fnode_t *parent = fs_parent_of(fnode);
	if (fs_filesys_of_fnode(parent) != ki_devio_filesys)
		return KM_RESULT_INVALID_ARGS;

	if (fs_get_fnode_type(fnode) != FS_FNODE_TYPE_FILE)
		return KM_RESULT_INVALID_ARGS;

	{
		ki_devio_fnode_exdata_t *base_exdata = (ki_devio_fnode_exdata_t *)fs_get_fnode_exdata(parent);
		ki_devio_dir_exdata_t *parent_exdata = (ki_devio_dir_exdata_t *)fs_get_fnode_exdata(parent);

		if (base_exdata->prev) {
			ki_devio_fnode_exdata_t *prev_exdata = (ki_devio_fnode_exdata_t *)fs_get_fnode_exdata(base_exdata->prev);
			prev_exdata->next = base_exdata->next;
		}

		if (base_exdata->next) {
			ki_devio_fnode_exdata_t *next_exdata = (ki_devio_fnode_exdata_t *)fs_get_fnode_exdata(base_exdata->next);
			next_exdata->prev = base_exdata->prev;
		}

		if (parent_exdata->first_child == fnode) {
			parent_exdata->first_child = base_exdata->next;
		}
	}

	KM_RETURN_IF_FAILED(fs_unlink_subnode(fnode));

	return KM_RESULT_OK;
}
