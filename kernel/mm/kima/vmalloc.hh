#ifndef _KIMA_VMALLOC_H_
#define _KIMA_VMALLOC_H_

#include <stdint.h>
#include <pbos/km/assert.h>
#include <pbos/mm/mm.h>
#include <pbos/kn/mm/mm.hh>

void *kima_vpgalloc(void *addr, size_t size);
void kima_vpgfree(void *addr, size_t size);

#endif
