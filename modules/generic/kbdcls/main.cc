#include <pbkxrt/init.h>
#include <pbos/dm/device.h>
#include <pbos/iodev/kbd.h>
#include <pbos/kd/logger.h>
#include <pbos/dm/device.hh>
#include <pbos/kfxx/scope_guard.hh>
#include "common.h"

PBOS_EXTERN_C_BEGIN

PBOS_USED PBOS_KMOD_API char PBOS_MODULE_NAME[] = IODEV_KBD_KMOD_NAME;

kfxx::rbtree_t<uint16_t> kbdcls_registered_devices;

dm_device_ops_t kbdcls_kbd_device_ops = {

};

km_result_t kbd_register_device(dm_device_t *device, const kbd_device_ops_t *ops, dm_device_t **device_out) {
	kfxx::unique_ptr<kbdcls_device_t, kfxx::deallocable_deleter<kbdcls_device_t>> dev(kbdcls_device_t::alloc());

	// KM_RETURN_IF_FAILED(dm_create_device(/* TODO: Fill it. */, &kbdcls_kbd_device_ops, dev.get_addr_without_release()));

	if (!dev)
		return KM_RESULT_NO_MEM;

	if (!kbdcls_alloc_kbd_id_and_insert(dev.get()))
		return KM_RESULT_NO_SLOT;

	kfxx::scope_guard g([&dev]() noexcept {
		kbdcls_remove_registered_device(dev.get());
	});

	fs::fnode_ptr fnode;

	char name[sizeof("0000")] = {};

	name[0] = (dev->rb_value & 0xff) + '0';
	name[1] = ((dev->rb_value >> 4) & 0xff) + '0';
	name[2] = ((dev->rb_value >> 8) & 0xff) + '0';
	name[3] = ((dev->rb_value >> 12) & 0xff) + '0';

	KM_RETURN_IF_FAILED(dm_create_devio_file(device, kbdcls_device_dir.get(), name, sizeof(name) - 1, fnode.get_addr_without_release()));

	g.release();

	return KM_RESULT_OK;
}

void kbd_unregister_device(dm_device_t *kbd_device) {
	// auto exdata = static_cast<kbdcls_device_exdata_t *>(dm_get_device_exdata(kbd_device));
	// fs::fnode_ptr fnode = exdata->device_file;

	// km_unwrap_result(dm_remove_devio_fnode(fnode.release()));

	dm_invalidate_device(kbd_device);
	dm_unref_device(kbd_device);

	// exdata->device_file.reset();
}

PBOS_USED PBOS_KMOD_API km_result_t pbos_module_init() {
	kxi_call_ctors();

	return KM_RESULT_OK;
}

PBOS_USED PBOS_KMOD_API void pbos_module_deinit() {
	kxi_call_dtors();
}

PBOS_EXTERN_C_END
