#include <pbos/ki/mm/kima.hh>
#include <pbos/km/logger.h>

void *kima_vpgalloc(void *addr, size_t size) {
	kd_assert(size);
	char *vaddr = (char *)mm_kvmalloc(mm_get_cur_context(), size, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, 0);
	kd_assert(vaddr);
	for (size_t i = 0; i < PGCEIL(size); i += PAGESIZE) {
		void *paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);
		if (!paddr)
			return nullptr;
		if (KM_FAILED(mm_mmap(mm_get_cur_context(), vaddr + i, paddr, PAGESIZE, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, 0)))
			kd_assert(false);
	}
	return vaddr;
}

void kima_vpgfree(void *addr, size_t size) {
	kd_assert(size);
	for (size_t i = 0; i < PGCEIL(size); i += PAGESIZE) {
		void *paddr = mm_getmap(mm_get_cur_context(), ((char *)addr) + i, NULL),
			 *vaddr = ((char *)addr) + i;
		mm_pgfree(paddr);
		mm_vmfree(mm_get_cur_context(), vaddr, size);
	}
}
