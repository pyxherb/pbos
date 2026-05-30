#include <pbos/ki/mm/kima.hh>
#include <pbos/kd/logger.h>

void *kima_vpgalloc(kima_pool_t *pool, void *addr, size_t size) {
	kd_assert(size);
	char *vaddr = (char *)mm_kvmalloc(mm_get_cur_context(), size, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, 0);
	kd_assert(vaddr);
	for (size_t i = 0; i < size; i += pool->page_size) {
		void *paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);
		if (!paddr)
			return nullptr;
		if (KM_FAILED(mm_mmap(mm_get_cur_context(), vaddr + i, paddr, pool->page_size, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, MM_MMAP_NO_INC_RC)))
			kd_assert(false);
	}
	return vaddr;
}

void kima_vpgfree(kima_pool_t *pool, void *addr, size_t size) {
	kd_assert(size);
	for (size_t i = 0; i < size; i += pool->page_size) {
		void *paddr = mm_getmap(mm_get_cur_context(), ((char *)addr) + i, NULL),
			 *vaddr = ((char *)addr) + i;
		mm_vmfree(mm_get_cur_context(), vaddr, pool->page_size);
	}
}
