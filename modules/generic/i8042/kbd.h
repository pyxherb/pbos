#ifndef _I8042_KBD_H_
#define _I8042_KBD_H_

#include "common.h"

PBOS_EXTERN_C_BEGIN

enum {
	I8042_KBD_BUFFER_MAX = 256
};

extern char i8042_kbd_buffer[I8042_KBD_BUFFER_MAX];

PBOS_EXTERN_C_END

#endif
