///
/// @file dispatch.h
/// @author PbOS Contributors
/// @brief Declarations for I/O dispatching functions.
///
/// @copyright Copyright (c) 2026 PbOS Project
///
#ifndef _PBOS_IO_DISPATCH_H_
#define _PBOS_IO_DISPATCH_H_

#include "iocb.h"

PBOS_EXTERN_C_BEGIN

typedef struct _io_dispatch_context_t {
	io_iocb_t *first_iocb, *last_iocb;
	size_t num_iocbs;
	size_t reserved[256 - sizeof(io_iocb_t) * 2 - sizeof(size_t)];
} io_dispatch_context_t;

PBOS_API void io_init_dispatch_context(io_dispatch_context_t *dc);
PBOS_API void io_push_iocb(io_dispatch_context_t *dc, io_iocb_t *iocb);
PBOS_API void io_pop_iocb(io_dispatch_context_t *dc);
PBOS_API io_iocb_t *io_current_iocb(io_dispatch_context_t *dc);
PBOS_API size_t io_iocb_list_size(io_dispatch_context_t *dc);
PBOS_API bool io_is_iocb_list_empty(io_dispatch_context_t *dc);

PBOS_EXTERN_C_END

#endif
