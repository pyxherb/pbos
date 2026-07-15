#include <pbkxrt/init.h>
#include <pbos/dm/device.h>
#include <pbos/iodev/kbd.h>
#include <pbos/kd/logger.h>
#include <pbos/kfxx/rbtree.hh>
#include "common.h"

PBOS_EXTERN_C_BEGIN

PBOS_USED PBOS_KMOD_API char PBOS_MODULE_NAME[] = IODEV_KBD_KMOD_NAME;

kfxx::rbtree_t<uint32_t> kbdcls_registered_keyboards;

km_result_t kbd_register_device(dm_device_t *device, const kbd_device_ops_t *ops, dm_device_t **device_out) {
	return KM_RESULT_OK;
}

void kbd_unregister_device(dm_device_t *kbd_device) {
	auto exdata = static_cast<kbdcls_device_exdata_t *>(dm_get_device_exdata(kbd_device));
	fs::fnode_ptr fnode = exdata->device_file;

	km_unwrap_result(dm_remove_devio_fnode(fnode.release()));

	dm_invalidate_device(kbd_device);
	dm_unref_device(kbd_device);

	exdata->device_file.reset();
}

PBOS_USED PBOS_KMOD_API km_result_t pbos_module_init() {
	kxi_call_ctors();

	return KM_RESULT_OK;
}

PBOS_USED PBOS_KMOD_API void pbos_module_deinit() {
	kxi_call_dtors();
}

PBOS_EXTERN_C_END
