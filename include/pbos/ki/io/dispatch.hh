#ifndef _PBOS_KI_IO_DISPATCH_H_
#define _PBOS_KI_IO_DISPATCH_H_

#include <pbos/io/dispatch.h>

PBOS_EXTERN_C_BEGIN

PBOS_FORCEINLINE bool ki_is_ctbs_list_empty(io_dispatch_context_t *dc) {
	return dc->num_ctbs;
}
PBOS_API km_result_t ki_poll_ctbs(io_dispatch_context_t *dc);

PBOS_EXTERN_C_END

#endif
