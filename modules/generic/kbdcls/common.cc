#include "common.h"

PBOS_EXTERN_C_BEGIN

fs::fnode_ptr kbdcls_device_dir;

kfxx::rbtree_t<uint16_t> kbdcls_registered_devices;
ps::mutex_t kbdcls_device_tree_mutex;

bool kbdcls_alloc_kbd_id_and_insert(kbdcls_device_t *device) {
	ps::mutex_guard g(kbdcls_device_tree_mutex);

	if (kbdcls_registered_devices.size() == UINT16_MAX)
		return false;

	if (kbdcls_registered_devices.empty()) {
		device->rb_value = 0;
		kbdcls_registered_devices.insert_unwrap(device);
		return true;
	}

	auto end = kbdcls_registered_devices.end_const();
	auto it = kbdcls_registered_devices.begin_const(), last_it = it;
	for (; it != end; ++it) {
		if ((*it) > (*last_it) + 1) {
			device->rb_value = (*last_it) + 1;
			kbdcls_registered_devices.insert_unwrap(device);
			return true;
		}
		last_it = it;
	}

	device->rb_value = (*last_it) + 1;
	kbdcls_registered_devices.insert_unwrap(device);
	return true;
}

void kbdcls_remove_registered_device(kbdcls_device_t *device) {
	kbdcls_registered_devices.remove(device);
}

PBOS_EXTERN_C_END
