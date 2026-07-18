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

void *mm_pgalloc(uint8_t memtype) {
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

			if (addr == (void *)0x0000000000e44000)
				mad = mad;

			if ((!(mad->pin_count++)) && (!mad->ref_count)) {
				i->free_list = mad->next_free;
				if (mad->prev_free)
					mad->prev_free->next_free = mad->next_free;
				if (mad->next_free)
					mad->next_free->prev_free = mad->prev_free;
				mad->prev_free = nullptr;
				mad->next_free = nullptr;
			}

			return addr;
		}
	}

	return NULL;
}

void mm_ref_page(void *ptr) {
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
	}
}

void mm_pin_page(void *ptr) {
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
	}
}

void mm_unref_page(void *ptr) {
	ki_pmad_t *area = ki_get_pmad(ptr);
	ki_mad_t *mad = kh_get_mad(ptr);

	if (ptr == (void *)0x0000000000e44000)
		mad = mad;

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
	}
}

void mm_unpin_page(void *ptr) {
	ki_pmad_t *area = ki_get_pmad(ptr);
	ki_mad_t *mad = kh_get_mad(ptr);

	if (ptr == (void *)0x0000000000e44000)
		mad = mad;

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
