#include "vmalloc.h"
#include <pbos/km/logger.h>

void *kima_vpgalloc(void *addr, size_t size) {
	char *vaddr = (char *)mm_kvmalloc(mm_kernel_context, size, PAGE_READ | PAGE_WRITE, 0);
    assert(vaddr);
	for (size_t i = 0; i < PGCEIL(size); i += PAGESIZE) {
		void *paddr = mm_pgalloc(MM_PMEM_AVAILABLE);
		assert(paddr);
		mm_mmap(mm_kernel_context, vaddr + i, paddr, PAGESIZE, PAGE_READ | PAGE_WRITE, 0);
	}
	return vaddr;
}

void kima_vpgfree(void *addr, size_t size) {
	for (size_t i = 0; i < PGCEIL(size); i += PAGESIZE) {
		void *paddr = mm_getmap(mm_kernel_context, ((char *)addr) + i),
			 *vaddr = ((char *)addr) + i;
        mm_pgfree(paddr);
		mm_vmfree(mm_kernel_context, vaddr, size);
	}
}