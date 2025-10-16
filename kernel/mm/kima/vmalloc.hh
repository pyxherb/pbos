#ifndef _KIMA_VMALLOC_H_
#define _KIMA_VMALLOC_H_

#include <stdint.h>
#include <arch/i386/paging.h>
#include <hal/i386/mm/vmmgr/vm.h>

void *kima_vpgalloc(void *addr, size_t size);
void kima_vpgfree(void *addr, size_t size);

#endif
