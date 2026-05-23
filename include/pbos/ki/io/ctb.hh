#ifndef _PBOS_KI_IO_CTB_H_
#define _PBOS_KI_IO_CTB_H_

#include <pbos/io/ctb.h>
#include <pbos/kf/misc.h>

PBOS_NODISCARD PBOS_API km_result_t io_alloc_ctb(io_ctb_ops_t *ops);
PBOS_NODISCARD PBOS_API void io_destroy_ctb(io_ctb_t *ctb);

#endif
