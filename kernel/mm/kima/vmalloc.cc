#include <pbos/kd/logger.h>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/mm/kima.hh>

void *kima_vpgalloc(kima_pool_t *pool, void *addr, size_t size) {
	kd_assert(size);
	char *vaddr = (char *)mm_kvmalloc(mm_get_cur_context(), size, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, 0);
	kd_assert(vaddr);
	kfxx::scope_guard release_vaddr_guard([vaddr, size]() noexcept {
		mm_vmfree(mm_get_cur_context(), vaddr, size);
	});
	for (size_t i = 0; i < size; i += pool->page_size) {
		void *paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);
		if (!paddr)
			return nullptr;
		kfxx::deferred unpin_guard([paddr]() noexcept {
			mm_unpin_page(paddr);
		});
		if (KM_FAILED(mm_mmap(mm_get_cur_context(), vaddr + i, paddr, pool->page_size, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, 0)))
			kd_assert(false);
	}
	pool->num_allocated_pages += kfxx::ceil_align_to(size, pool->page_size);
	return vaddr;
}

void kima_vpgfree(kima_pool_t *pool, void *addr, size_t size) {
	kd_assert(size);
	for (size_t i = 0; i < size; i += pool->page_size) {
		void *paddr = mm_getmap(mm_get_cur_context(), ((char *)addr) + i, NULL),
			 *vaddr = ((char *)addr) + i;
		mm_vmfree(mm_get_cur_context(), vaddr, pool->page_size);
	}
	pool->num_allocated_pages -= kfxx::ceil_align_to(size, pool->page_size);
}
