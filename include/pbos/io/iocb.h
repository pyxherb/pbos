///
/// @file iocb.h
/// @author PbOS Contributors
/// @brief Declarations for I/O Continuation Block (IOCB).
///
/// @copyright Copyright (c) 2026 PbOS Project
///
#ifndef _PBOS_IO_CTB_H_
#define _PBOS_IO_CTB_H_

#include <pbos/km/result.h>
#include <stdalign.h>

PBOS_EXTERN_C_BEGIN

typedef struct _io_dispatch_context_t io_dispatch_context_t;
typedef struct _io_iocb_t io_iocb_t;

typedef km_result_t (*io_iocb_perform_fn_t)(io_dispatch_context_t *dispatch_context, io_iocb_t *iocb);
typedef km_result_t (*io_iocb_finish_fn_t)(io_dispatch_context_t *dispatch_context, io_iocb_t *iocb, km_result_t result);
typedef void (*io_iocb_fail_fn_t)(io_dispatch_context_t *dispatch_context, io_iocb_t *iocb, km_result_t result);
typedef void (*io_iocb_destroy_fn_t)(io_iocb_t *iocb);

typedef struct _io_iocb_ops_t {
	io_iocb_perform_fn_t perform;
	io_iocb_finish_fn_t finish;
	io_iocb_fail_fn_t fail;
	io_iocb_destroy_fn_t destroy;
} io_iocb_ops_t;

typedef struct _io_iocb_t {
	alignas(max_align_t) size_t size;
	io_iocb_t *prev, *next;
	io_iocb_ops_t ops;
} io_iocb_t;

PBOS_NODISCARD PBOS_API km_result_t io_alloc_iocb(io_iocb_ops_t *ops, size_t exdata_size, io_iocb_t **iocb_out);
PBOS_API void io_destroy_iocb(io_iocb_t *iocb);
PBOS_API void *io_get_iocb_exdata(io_iocb_t *iocb);

PBOS_EXTERN_C_END

#endif
