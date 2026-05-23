#ifndef _PBOS_IO_CTB_H_
#define _PBOS_IO_CTB_H_

#include <pbos/km/result.h>
#include <stdalign.h>

PBOS_EXTERN_C_BEGIN

typedef struct _io_dispatch_context_t io_dispatch_context_t;
typedef struct _io_ctb_t io_ctb_t;

typedef km_result_t (*io_ctb_perform_fn_t)(io_dispatch_context_t *dispatch_context, io_ctb_t *ctb);
typedef km_result_t (*io_ctb_finish_fn_t)(io_dispatch_context_t *dispatch_context, io_ctb_t *ctb, km_result_t result);
typedef void (*io_ctb_fail_fn_t)(io_dispatch_context_t *dispatch_context, io_ctb_t *ctb, km_result_t result);
typedef void (*io_ctb_destroy_fn_t)(io_ctb_t *ctb);

typedef struct _io_ctb_ops_t {
	io_ctb_perform_fn_t perform;
	io_ctb_finish_fn_t finish;
	io_ctb_fail_fn_t fail;
	io_ctb_destroy_fn_t destroy;
} io_ctb_ops_t;

/// @brief The CTB (Continuable Task Block) structure.
typedef struct _io_ctb_t {
	alignas(max_align_t) size_t size;
	io_ctb_t *prev, *next;
	io_ctb_ops_t ops;
} io_ctb_t;

PBOS_NODISCARD PBOS_API km_result_t io_alloc_ctb(io_ctb_ops_t *ops, size_t exdata_size, io_ctb_t **ctb_out);
PBOS_API void io_destroy_ctb(io_ctb_t *ctb);
PBOS_API void *io_get_ctb_exdata(io_ctb_t *ctb);

PBOS_EXTERN_C_END

#endif
