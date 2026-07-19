#ifndef _PBOS_IO_CONTEXT_H_
#define _PBOS_IO_CONTEXT_H_

#include "irq.h"

typedef struct _io_context_t io_context_t;

PBOS_API io_context_t *io_get_cur_context();

#endif
