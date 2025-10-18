#ifndef _HAL_I386_MM_IOPGALLOC_HH_
#define _HAL_I386_MM_IOPGALLOC_HH_

#include <arch/i386/paging.h>
#include <pbos/km/mm.h>
#include <pbos/kfxx/rbtree.hh>
#include "../vmmgr/vm.h"

PBOS_EXTERN_C_BEGIN

#define MAD_P 0x01	// Present
#define MAD_S 0x02	// Shared
#define MAD_C 0x04	// Cached
#define MAD_M 0x08	// Mapped
#define MAD_L 0x10	// Locked
#define MAD_D 0x20	// Dirty
#define MAD_B 0x40	// Busy

///
/// @brief I/O Mapping Descriptor (IOMD), manages a single page.
///
typedef struct _hn_iomd_t : public kfxx::rbtree_t<pgaddr_t>::node_t {
	uint32_t ref_count;

	uint32_t mapped_pgtab_addr : 20;

	uint8_t flags : 8;
	uint8_t type : 8;
} hn_iomd_t;

extern kfxx::rbtree_t<pgaddr_t> iomd_query_tree;

PBOS_EXTERN_C_END

#endif
