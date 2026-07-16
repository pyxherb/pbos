#ifndef _KBDCLS_COMMON_H_
#define _KBDCLS_COMMON_H_

#include <pbos/iodev/kbd.h>
#include <pbos/dm/device.hh>
#include <pbos/fs/file.hh>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/ps/mutex.hh>

PBOS_EXTERN_C_BEGIN

struct kbdcls_device_t : public kfxx::rbtree_t<uint16_t>::node_t {
	fs::fnode_ptr kbd_file;
	dm::device_ptr source_device;
	kbd_device_ops_t ops;

	PBOS_FORCEINLINE kbdcls_device_t(const kbd_device_ops_t &ops) : ops(ops) {
	}

	PBOS_FORCEINLINE void dealloc() {
		kfxx::destroy_and_release<kbdcls_device_t>(kfxx::kernel_allocator(), this);
	}

	PBOS_FORCEINLINE static kbdcls_device_t *alloc(const kbd_device_ops_t &ops) {
		return kfxx::alloc_and_construct<kbdcls_device_t>(kfxx::kernel_allocator(), ops);
	}
};

extern fs::fnode_ptr kbdcls_device_dir;

extern kfxx::rbtree_t<uint16_t> kbdcls_registered_devices;
extern ps::mutex_t kbdcls_device_tree_mutex;

extern dm_device_class_t *kbdcls_keyboard_device_class;

bool kbdcls_alloc_kbd_id_and_insert(kbdcls_device_t *device);
void kbdcls_remove_registered_device(kbdcls_device_t *device);

PBOS_EXTERN_C_END

#endif
