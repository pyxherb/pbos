#ifndef _PBOS_KI_IO_CTB_H_
#define _PBOS_KI_IO_CTB_H_

#include <pbos/io/iocb.h>
#include <pbos/kf/misc.h>

PBOS_NODISCARD PBOS_API km_result_t io_alloc_iocb(io_iocb_ops_t *ops);
PBOS_NODISCARD PBOS_API void io_destroy_iocb(io_iocb_t *iocb);

#endif
