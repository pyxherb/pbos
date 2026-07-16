#include <pbkxrt/init.h>
#include <pbos/dm/device.h>
#include <pbos/iodev/kbd.h>
#include <pbos/kd/logger.h>
#include <pbos/dm/device.hh>
#include <pbos/kfxx/scope_guard.hh>
#include "devio.h"

PBOS_EXTERN_C_BEGIN

PBOS_USED PBOS_KMOD_API char PBOS_MODULE_NAME[] = IODEV_KBD_KMOD_NAME;

kfxx::rbtree_t<uint16_t> kbdcls_registered_devices;

dm_devio_file_ops_t kbdcls_kbd_file_ops = {
	.open = kbdcls_devio_open,
	.close = kbdcls_devio_close,

	.remove = kbdcls_devio_remove,

	.seek = nullptr,

	.read = kbdcls_devio_read,
	.write = kbdcls_devio_write,

	.pread = kbdcls_devio_pread,
	.pwrite = kbdcls_devio_pwrite,

	.ioctl = kbdcls_devio_ioctl,

	.destroy = kbdcls_devio_destroy
};

km_result_t kbd_register_device(dm_device_t *device, const kbd_device_ops_t *ops, kbd_device_id_t *device_id_out, fs_fnode_t **fnode_out) {
	kfxx::unique_ptr<kbdcls_device_t, kfxx::deallocable_deleter<kbdcls_device_t>> kbdcls_dev(kbdcls_device_t::alloc());

	if (!kbdcls_dev)
		return KM_RESULT_NO_MEM;

	if (!kbdcls_alloc_kbd_id_and_insert(kbdcls_dev.get()))
		return KM_RESULT_NO_SLOT;

	kfxx::scope_guard g([&kbdcls_dev]() noexcept {
		kbdcls_remove_registered_device(kbdcls_dev.get());
	});

	fs::fnode_ptr fnode;

	char name[sizeof("0000")] = {};

	name[0] = (kbdcls_dev->rb_value & 0xff) + '0';
	name[1] = ((kbdcls_dev->rb_value >> 4) & 0xff) + '0';
	name[2] = ((kbdcls_dev->rb_value >> 8) & 0xff) + '0';
	name[3] = ((kbdcls_dev->rb_value >> 12) & 0xff) + '0';

	KM_RETURN_IF_FAILED(dm_create_devio_file(device, kbdcls_device_dir.get(), name, sizeof(name) - 1, &kbdcls_kbd_file_ops, fnode.get_addr_without_release()));

	fs_set_fnode_exdata(fnode.get(), kbdcls_dev.release());

	g.release();

	*device_id_out = kbdcls_dev->rb_value;
	*fnode_out = fnode.release();

	return KM_RESULT_OK;
}

km_result_t kbd_unregister_device(uint16_t device_id) {
	ps::mutex_guard g(kbdcls_device_tree_mutex);
	kbdcls_device_t *dev;

	if (auto it = kbdcls_registered_devices.find(device_id); it)
		dev = static_cast<kbdcls_device_t *>(dev);
	else
		return KM_RESULT_NOT_FOUND;

	{
		fs::fnode_ptr fnode = dev->kbd_file;

		km_unwrap_result(dm_remove_devio_fnode(fnode.release()));
	}

	kbdcls_remove_registered_device(dev);

	dev->dealloc();

	return KM_RESULT_OK;
}

PBOS_USED PBOS_KMOD_API km_result_t pbos_module_init() {
	kxi_call_ctors();

	{
		kf_uuid_t uuid = DM_DEVICE_CLASS_KEYBOARD;

		KM_RETURN_IF_FAILED(dm_register_device_class(&uuid, &kbdcls_keyboard_device_class));
	}

	return KM_RESULT_OK;
}

PBOS_USED PBOS_KMOD_API void pbos_module_deinit() {
	kxi_call_dtors();
}

PBOS_EXTERN_C_END
