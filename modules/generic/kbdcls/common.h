#ifndef _KBDCLS_COMMON_H_
#define _KBDCLS_COMMON_H_

#include <pbos/iodev/kbd.h>
#include <pbos/fs/file.hh>

typedef struct _kbdcls_device_exdata_t {
	uint16_t device_id;
	fs::fnode_ptr device_file;
	kbd_device_ops_t ops;
} kbdcls_device_exdata_t;

extern fs::fnode_ptr kbdcls_device_dir;

#endif
