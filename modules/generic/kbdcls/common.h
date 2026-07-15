#ifndef _KBDCLS_COMMON_H_
#define _KBDCLS_COMMON_H_

#include <pbos/iodev/kbd.h>

typedef struct _kbd_device_exdata_t {
	kbd_device_ops_t ops;
} kbd_device_exdata_t;

#endif
