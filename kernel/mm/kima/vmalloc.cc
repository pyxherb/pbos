#include "vmalloc.hh"
#include <pbos/km/logger.h>

void *kima_vpgalloc(void *addr, size_t size) {
	kd_assert(size);
	char *vaddr = (char *)mm_kvmalloc(mm_kernel_context, size, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, 0);
	kd_assert(vaddr);
	for (size_t i = 0; i < PGCEIL(size); i += PAGESIZE) {
		void *paddr = mm_pgalloc(MM_PMEM_AVAILABLE);
		kd_assert(paddr);
		if (KM_FAILED(mm_mmap(mm_kernel_context, vaddr + i, paddr, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, 0)))
			kd_assert(false);
	}
	return vaddr;
}

void kima_vpgfree(void *addr, size_t size) {
	kd_assert(size);
	for (size_t i = 0; i < PGCEIL(size); i += PAGESIZE) {
		void *paddr = mm_getmap(mm_kernel_context, ((char *)addr) + i, NULL),
			 *vaddr = ((char *)addr) + i;
		mm_pgfree(paddr);
		mm_vmfree(mm_kernel_context, vaddr, size);
	}
}
