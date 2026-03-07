#ifndef _HAL_X86_64_MM_IOPGALLOC_HH_
#define _HAL_X86_64_MM_IOPGALLOC_HH_

#include <arch/x86_64/paging.h>
#include <pbos/mm/mm.h>
#include <pbos/kfxx/rbtree.hh>
#include "vmmgr/vm.h"

PBOS_EXTERN_C_BEGIN

///
/// @brief I/O Mapping Descriptor (IOMD), manages a single page.
///
typedef struct _hn_iomd_t : public kfxx::rbtree_t<pgaddr_t>::node_t {
	uint32_t ref_count;

	uint8_t flags : 8;
} hn_iomd_t;

PBOS_EXTERN_C_END

#endif
