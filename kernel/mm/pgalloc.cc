#include <pbos/kf/atomic.h>
#include <pbos/io/irq.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/mm/pgalloc.hh>

PBOS_EXTERN_C_BEGIN

ki_pmad_t ki_initial_pmad_storage[KI_INITIAL_MM_AREA_STORAGE_NUM];
kfxx::rbtree_t<void *> ki_pmad_tree;
size_t ki_pmad_number;
ki_madpool_t *ki_global_mad_pool_list = NULL;

size_t kh_mad_pool_descs_off, kh_mad_pool_descs_num_per_page;

ki_pmad_t::ki_pmad_t() {
}

// Note: we initialized the page allocation counter with the maximum value in the early stage to avoid the uninitialized condition.
size_t ki_num_available_phy_pages = SIZE_MAX, ki_num_total_free_pages = SIZE_MAX;
ps::semaphore_t ki_page_alloc_counter_semaphore;

void ki_init_page_alloc_counter() {
	ki_num_available_phy_pages = 0;
	ki_num_total_free_pages = 0;

	size_t page_size = mm_get_page_size();

	KI_PMAD_FOREACH(i) {
		if (i->type != MM_PHYSICAL_MEMORY_TYPE_AVAILABLE)
			continue;

		ki_num_available_phy_pages += i->len;
		ki_num_total_free_pages += (i->len / page_size) - i->used_count;
	}
}

PBOS_API void *mm_alloc_single_page(uint8_t memtype) {
	{
		ps::write_semaphore_guard g(ki_page_alloc_counter_semaphore);

		if (ki_num_total_free_pages < 1)
			return nullptr;

		--ki_num_total_free_pages;
	}

	KI_PMAD_FOREACH(i) {
		if (i->type != memtype)
			continue;

		hal_lock_spinlock(&i->spinlock);
		kfxx::deferred unlocker([i]() noexcept {
			hal_unlock_spinlock(&i->spinlock);
		});

		if (i->free_list) {
			void *addr = i->free_list->rb_value;
			ki_mad_t *mad = i->free_list;

			mad->pin_count++;
			// if ((!(mad->pin_count++)) && (!mad->ref_count)) {
			i->free_list = mad->next_free;
			if (mad->prev_free)
				mad->prev_free->next_free = mad->next_free;
			if (mad->next_free)
				mad->next_free->prev_free = mad->prev_free;
			mad->prev_free = nullptr;
			mad->next_free = nullptr;

			++i->used_count;
			// }

			return addr;
		}
	}

	return NULL;
}

PBOS_API km_result_t mm_alloc_pages(uint8_t memtype, void **pages_out, size_t num_pages) {
	{
		ps::write_semaphore_guard g(ki_page_alloc_counter_semaphore);

		if (ki_num_total_free_pages < num_pages)
			return KM_RESULT_NO_MEM;
		ki_num_total_free_pages -= num_pages;
	}

	size_t count = 0;
	KI_PMAD_FOREACH(i) {
		if (i->type != memtype)
			continue;

		hal_lock_spinlock(&i->spinlock);
		kfxx::deferred unlocker([i]() noexcept {
			hal_unlock_spinlock(&i->spinlock);
		});

		if (i->free_list) {
			void *addr = i->free_list->rb_value;
			ki_mad_t *mad = i->free_list;

			mad->pin_count++;
			// if ((!(mad->pin_count++)) && (!mad->ref_count)) {
			i->free_list = mad->next_free;
			if (mad->prev_free)
				mad->prev_free->next_free = mad->next_free;
			if (mad->next_free)
				mad->next_free->prev_free = mad->prev_free;
			mad->prev_free = nullptr;
			mad->next_free = nullptr;

			++i->used_count;
			// }

			pages_out[count] = addr;
			++count;

			if (count >= num_pages)
				break;
		}
	}

	if (num_pages < count)
		km_panic("%s didn't get expected number of allocatable memory, please report this bug to developers", __func__);

	return KM_RESULT_OK;
}

PBOS_API km_result_t mm_reserve_pages(mm_context_t *context, size_t num_pages) {
	{
		ps::write_semaphore_guard g(ki_page_alloc_counter_semaphore);

		if (ki_num_total_free_pages < num_pages)
			return KM_RESULT_NO_MEM;

		ki_num_total_free_pages -= num_pages;
	}

	{
		ps::write_semaphore_guard g(context->user_page_reserve_quota_semaphore);
		context->num_reserved_user_pages += num_pages;
	}

	return KM_RESULT_OK;
}

void ki_unreserve_page_quota(mm_context_t *context, size_t num_pages) {
	{
		ps::write_semaphore_guard g(context->user_page_reserve_quota_semaphore);

		if (context->num_reserved_user_pages < num_pages)
			km_panic("Context does not have enough user pages to unreserve, please report this bug to developers");

		context->num_reserved_user_pages += num_pages;
	}

	{
		ps::write_semaphore_guard g(ki_page_alloc_counter_semaphore);

		if (ki_num_total_free_pages < num_pages)
			km_panic("Mismatched total free page counter, please report this bug to developers");

		ki_num_total_free_pages -= num_pages;
	}
}

PBOS_API void *mm_commit_single_reserved_page(mm_context_t *context) {
	{
		ps::write_semaphore_guard g(context->user_page_reserve_quota_semaphore);
		if (!context->num_reserved_user_pages)
			return nullptr;
		--context->num_reserved_user_pages;
	}

	KI_PMAD_FOREACH(i) {
		if (i->type != MM_PHYSICAL_MEMORY_TYPE_AVAILABLE)
			continue;

		hal_lock_spinlock(&i->spinlock);
		kfxx::deferred unlocker([i]() noexcept {
			hal_unlock_spinlock(&i->spinlock);
		});

		if (i->free_list) {
			void *addr = i->free_list->rb_value;
			ki_mad_t *mad = i->free_list;

			mad->pin_count++;
			// if ((!(mad->pin_count++)) && (!mad->ref_count)) {
			i->free_list = mad->next_free;
			if (mad->prev_free)
				mad->prev_free->next_free = mad->next_free;
			if (mad->next_free)
				mad->next_free->prev_free = mad->prev_free;
			mad->prev_free = nullptr;
			mad->next_free = nullptr;

			++i->used_count;
			// }

			return addr;
		}
	}

	km_panic("Unable to find any free page with free page committed, please report this bug to developers");
}

PBOS_NODISCARD PBOS_API km_result_t mm_commit_reserved_pages(mm_context_t *context, void **pages_out, size_t num_pages) {
	{
		ps::write_semaphore_guard g(context->user_page_reserve_quota_semaphore);
		if (context->num_reserved_user_pages < num_pages)
			return KM_RESULT_NO_MEM;
		context->num_reserved_user_pages -= num_pages;
	}

	size_t count = 0;
	KI_PMAD_FOREACH(i) {
		if (i->type != MM_PHYSICAL_MEMORY_TYPE_AVAILABLE)
			continue;

		hal_lock_spinlock(&i->spinlock);
		kfxx::deferred unlocker([i]() noexcept {
			hal_unlock_spinlock(&i->spinlock);
		});

		if (i->free_list) {
			void *addr = i->free_list->rb_value;
			ki_mad_t *mad = i->free_list;

			mad->pin_count++;
			i->free_list = mad->next_free;
			if (mad->prev_free)
				mad->prev_free->next_free = mad->next_free;
			if (mad->next_free)
				mad->next_free->prev_free = mad->prev_free;
			mad->prev_free = nullptr;
			mad->next_free = nullptr;

			pages_out[count] = addr;
			++count;

			if (count >= num_pages)
				break;
		}
	}

	if (count < num_pages)
		km_panic("%s didn't get committed number of pages, please report this bug to developers", __func__);

	return KM_RESULT_OK;
}

PBOS_API void mm_ref_page(void *ptr) {
	ki_pmad_t *area = ki_get_pmad(ptr);
	ki_mad_t *mad = kh_get_mad(ptr);

	hal_lock_spinlock(&mad->spinlock);
	kfxx::deferred unlocker([mad]() noexcept {
		hal_unlock_spinlock(&mad->spinlock);
	});

	if ((!(mad->ref_count++)) && (!mad->pin_count)) {
		hal_lock_spinlock(&area->spinlock);
		kfxx::deferred unlocker([area]() noexcept {
			hal_unlock_spinlock(&area->spinlock);
		});

		if (mad == area->free_list)
			area->free_list = mad->next_free;
		if (mad->prev_free)
			mad->prev_free->next_free = mad->next_free;
		if (mad->next_free)
			mad->next_free->prev_free = mad->prev_free;
		mad->prev_free = nullptr;
		mad->next_free = nullptr;

		ps::write_semaphore_guard g(ki_page_alloc_counter_semaphore);
		--ki_num_total_free_pages;
		++area->used_count;
	}
}

PBOS_API void mm_pin_page(void *ptr) {
	ki_pmad_t *area = ki_get_pmad(ptr);
	ki_mad_t *mad = kh_get_mad(ptr);

	hal_lock_spinlock(&mad->spinlock);
	kfxx::deferred unlocker([mad]() noexcept {
		hal_unlock_spinlock(&mad->spinlock);
	});

	if ((!(mad->pin_count++)) && (!mad->ref_count)) {
		hal_lock_spinlock(&area->spinlock);
		kfxx::deferred unlocker([area]() noexcept {
			hal_unlock_spinlock(&area->spinlock);
		});

		if (mad == area->free_list)
			area->free_list = mad->next_free;
		if (mad->prev_free)
			mad->prev_free->next_free = mad->next_free;
		if (mad->next_free)
			mad->next_free->prev_free = mad->prev_free;
		mad->prev_free = nullptr;
		mad->next_free = nullptr;

		{
			ps::write_semaphore_guard g(ki_page_alloc_counter_semaphore);
			--ki_num_total_free_pages;
		}
		++area->used_count;
	}
}

PBOS_API void mm_unref_page(void *ptr) {
	ki_pmad_t *area = ki_get_pmad(ptr);
	ki_mad_t *mad = kh_get_mad(ptr);

	hal_lock_spinlock(&mad->spinlock);
	kfxx::deferred unlocker([mad]() noexcept {
		hal_unlock_spinlock(&mad->spinlock);
	});

	if (!mad->ref_count)
		km_panic("Freeing physical page at %p with reference count == 0", ptr);
	if ((!(--mad->ref_count)) && (!mad->pin_count)) {
		hal_lock_spinlock(&area->spinlock);
		kfxx::deferred unlocker([area]() noexcept {
			hal_unlock_spinlock(&area->spinlock);
		});

		if (area->free_list)
			area->free_list->prev_free = mad;
		mad->next_free = area->free_list;
		kd_assert(!mad->prev_free);

		ps::write_semaphore_guard g(ki_page_alloc_counter_semaphore);
		++ki_num_total_free_pages;
		--area->used_count;
	}
}

PBOS_API void mm_unpin_page(void *ptr) {
	ki_pmad_t *area = ki_get_pmad(ptr);
	ki_mad_t *mad = kh_get_mad(ptr);

	hal_lock_spinlock(&mad->spinlock);
	kfxx::deferred unlocker([mad]() noexcept {
		hal_unlock_spinlock(&mad->spinlock);
	});

	kd_dbgcheck(mad->pin_count, "Unpinning page %p that is not pinned", ptr);
	if ((!(--mad->pin_count)) && (!mad->ref_count)) {
		hal_lock_spinlock(&area->spinlock);
		kfxx::deferred unlocker([area]() noexcept {
			hal_unlock_spinlock(&area->spinlock);
		});

		if (area->free_list)
			area->free_list->prev_free = mad;
		mad->next_free = area->free_list;
		kd_assert(!mad->prev_free);

		ps::write_semaphore_guard g(ki_page_alloc_counter_semaphore);
		++ki_num_total_free_pages;
		--area->used_count;
	}
}

ki_mad_t *kh_get_mad(void *pgaddr) {
	ki_pmad_t *pmad = ki_get_pmad(pgaddr);
	if (!pmad)
		km_panic("No PMAD corresponds to physical address %p", pgaddr);

	kfxx::rbtree_t<void *>::node_t *mad;
	if ((mad = pmad->query_tree.find(pgaddr))) {
		return static_cast<ki_mad_t *>(mad);
	}

	km_panic("Physical memory block not found: %p", pgaddr);
}

///
/// @brief Get PMAD by a physical address.
///
/// @param addr Physical address to find.
/// @return Corresponding PMAD. NULL if not found.
///
ki_pmad_t *ki_get_pmad(void *addr) {
	auto node = ki_pmad_tree.find_max_lteq(addr);

	if (node) {
		if (((char *)node->rb_value) + (static_cast<ki_pmad_t *>(node)->len) > addr)
			return static_cast<ki_pmad_t *>(node);
	}

	return nullptr;
}

PBOS_EXTERN_C_END
