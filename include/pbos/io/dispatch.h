#ifndef _PBOS_IO_DISPATCH_H_
#define _PBOS_IO_DISPATCH_H_

#include "ctb.h"

PBOS_EXTERN_C_BEGIN

typedef struct _io_dispatch_context_t {
	io_ctb_t *first_ctb, *last_ctb;
	size_t num_ctbs;
	size_t reserved[256 - sizeof(io_ctb_t) * 2 - sizeof(size_t)];
} io_dispatch_context_t;

PBOS_API void io_init_dispatch_context(io_dispatch_context_t *dc);
PBOS_API void io_push_ctb(io_dispatch_context_t *dc, io_ctb_t *ctb);
PBOS_API void io_pop_ctb(io_dispatch_context_t *dc);
PBOS_API io_ctb_t *io_current_ctb(io_dispatch_context_t *dc);
PBOS_API size_t io_ctb_list_size(io_dispatch_context_t *dc);
PBOS_API bool io_is_ctb_list_empty(io_dispatch_context_t *dc);

PBOS_EXTERN_C_END

#endif
