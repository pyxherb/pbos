#ifndef _I8042_COMMON_H_
#define _I8042_COMMON_H_

#include <pbos/iodev/kbd.h>

#define I8042_KMOD_NAME "i8042"

typedef struct _kbd_device_exdata_t {
	kbd_device_ops_t ops;
} kbd_device_exdata_t;

#endif
