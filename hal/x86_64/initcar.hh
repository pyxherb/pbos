#ifndef _HAL_X86_64_INITCAR_H_
#define _HAL_X86_64_INITCAR_H_

#include <pbos/kh/initcar.hh>
#include <pbos/fs/fs.h>

PBOS_EXTERN_C_BEGIN

struct hn_initcar_file_exdata {
	const char *ptr;
	size_t sz_total;
};

extern void *hn_initcar_ptr;
extern void *hn_initcar_paddr;
extern size_t hn_initcar_size;
extern bool hn_is_initcar_direct_mapped;

PBOS_EXTERN_C_END

#endif
