#include <pbos/kf/atomic.h>
#include <pbos/kf/hash.h>
#include <string.h>
#include <hal/x86_64/proc.hh>
#include <pbos/kfxx/map.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/ps/exec.hh>
#include <pbos/kd/logger.h>
#include <pbos/ps/semaphore.hh>

PBOS_EXTERN_C_BEGIN

ki_ps_cached_ro_pages_buckets_allocator_t::ki_ps_cached_ro_pages_buckets_allocator_t() {
}

ki_ps_cached_ro_pages_buckets_allocator_t::~ki_ps_cached_ro_pages_buckets_allocator_t() {
}

size_t ki_ps_cached_ro_pages_buckets_allocator_t::inc_ref() noexcept {
	return 0;
}

size_t ki_ps_cached_ro_pages_buckets_allocator_t::dec_ref() noexcept {
	return 0;
}

void *ki_ps_cached_ro_pages_buckets_allocator_t::alloc(size_t size, size_t alignment) noexcept {
	return mm_kalloc(size, alignment);
}

void *ki_ps_cached_ro_pages_buckets_allocator_t::realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	return mm_krealloc(ptr, new_size, new_alignment);
}

void *ki_ps_cached_ro_pages_buckets_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size) noexcept {
	return mm_krealloc_in_place(ptr, new_size);
}

void ki_ps_cached_ro_pages_buckets_allocator_t::release(void *ptr, size_t size, size_t alignment) noexcept {
	mm_kfree(ptr);
}

int ki_ps_cached_ro_pages_buckets_allocator_identity;

void *ki_ps_cached_ro_pages_buckets_allocator_t::type_identity() const noexcept {
	return &ki_ps_cached_ro_pages_buckets_allocator_identity;
}

ki_ps_cached_ro_pages_buckets_allocator_t ki_ps_cached_ro_pages_buckets_allocator;

ki_ps_cached_ro_pages_registry_allocator_t::ki_ps_cached_ro_pages_registry_allocator_t() {
}

ki_ps_cached_ro_pages_registry_allocator_t::~ki_ps_cached_ro_pages_registry_allocator_t() {
}

size_t ki_ps_cached_ro_pages_registry_allocator_t::inc_ref() noexcept {
	return 0;
}

size_t ki_ps_cached_ro_pages_registry_allocator_t::dec_ref() noexcept {
	return 0;
}

void *ki_ps_cached_ro_pages_registry_allocator_t::alloc(size_t size, size_t alignment) noexcept {
	return mm_kalloc(size, alignment);
}

void *ki_ps_cached_ro_pages_registry_allocator_t::realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	return mm_krealloc(ptr, new_size, new_alignment);
}

void *ki_ps_cached_ro_pages_registry_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	// return mm_krealloc(ptr, new_size, new_alignment);
	return nullptr;
}

void ki_ps_cached_ro_pages_registry_allocator_t::release(void *ptr, size_t size, size_t alignment) noexcept {
	mm_kfree(ptr);
}

int ki_ps_cached_ro_pages_registry_allocator_identity;

void *ki_ps_cached_ro_pages_registry_allocator_t::type_identity() const noexcept {
	return &ki_ps_cached_ro_pages_registry_allocator_identity;
}

ki_ps_cached_ro_pages_registry_allocator_t ki_ps_cached_ro_pages_registry_allocator;

kfxx::rbtree_t<kf_uuid_t> ki_registered_binldrs;
kfxx::rbtree_t<fs_fcb_t *> ki_registered_binprotos;

static ps_proc_id_t _last_proc_id = 0;

ps_proc_id_t ki_alloc_proc_id() {
	return _last_proc_id++;
}

km_result_t ps_register_binldr(kf_uuid_t *uuid, km_binldr_ops_t *binldr) {
	if (ki_registered_binldrs.find(*uuid))
		return KM_RESULT_EXISTED;
	ki_binldr_registry_t *reg = kfxx::alloc_and_construct<ki_binldr_registry_t>(kfxx::kernel_allocator());
	if (!reg)
		return KM_RESULT_NO_MEM;

	memcpy(&reg->rb_value, &uuid, sizeof(*uuid));
	memcpy(&reg->ops, binldr, sizeof(km_binldr_ops_t));

	ki_registered_binldrs.insert_unwrap(reg);

	return KM_RESULT_OK;
}

km_result_t ps_exec(
	ps_proc_id_t parent,
	se_uid_t uid,
	fs_fcb_t *file_fp,
	ps_proc_id_t *pid_out) {
	km_result_t result;

	ps_proc_id_t new_proc_id = ki_alloc_proc_id();
	if (new_proc_id < 0)
		return KM_RESULT_NO_SLOT;

	ps_pcb_t *pcb = ps_alloc_pcb();
	if (!pcb) {
		return KM_RESULT_NO_MEM;
	}

	kfxx::scope_guard destroy_pcb_guard([pcb]() noexcept {
		ki_destroy_pcb(pcb);
	});

	for (auto it = ki_registered_binldrs.begin(); it != ki_registered_binldrs.end(); ++it) {
		if (KM_SUCCESS(result = static_cast<ki_binldr_registry_t *>(it.node)->ops.load_exec(pcb, file_fp))) {
			KM_RETURN_IF_FAILED(ps_create_proc(pcb, parent));
			destroy_pcb_guard.release();
			return KM_RESULT_OK;
		}

		if (result != KM_RESULT_UNSUPPORTED_EXECFMT)
			return result;
	}

	return KM_RESULT_UNSUPPORTED_EXECFMT;
}

km_result_t ps_register_binproto(fs_fcb_t *fcb, km_binproto_t **proto_out) {
	// TODO: Add a binproto lock.
	if (ki_registered_binprotos.find(fcb))
		return KM_RESULT_EXISTED;

	km_binproto_t *proto = (km_binproto_t *)mm_kalloc(sizeof(km_binproto_t), alignof(km_binproto_t));

	kfxx::construct_at<km_binproto_t>(proto);

	proto->rb_value = fcb;

	ki_registered_binprotos.insert_unwrap(proto);

	*proto_out = proto;

	return KM_RESULT_OK;
}

typedef struct _ki_cached_ro_page_registry : public kfxx::rbtree_t<void *>::node_t {
	size_t ref_count = 0;
} ki_cached_ro_page_registry;

kfxx::map_t<uint64_t, kfxx::rbtree_t<void *>> ki_cached_ro_pages_hash_map(&ki_ps_cached_ro_pages_buckets_allocator);
kfxx::map_t<void *, uint64_t> ki_cached_ro_pages_paddr_to_hash_map(&ki_ps_cached_ro_pages_buckets_allocator);
ps::semaphore_t ki_ro_pages_cache_lock;
// TODO: Add a read/write lock for the hash map.

km_result_t ps_register_cached_ro_page(void *paddr, void *allocated_cmp_vpage, void *vaddr) {
	const size_t page_size = mm_get_page_size();
	uint64_t hash_code = kf_djb_hash64((const char *)vaddr, page_size);

	ps::write_semaphore_guard g(ki_ro_pages_cache_lock);

	if (auto it = ki_cached_ro_pages_hash_map.find(hash_code); it != ki_cached_ro_pages_hash_map.end()) {
		// TODO: Implement this...
	}

	return KM_RESULT_OK;
}

km_result_t ps_fetch_cached_ro_page(void *vaddr, void *comparison_tmpmap_vaddr, void **paddr_out) {
	if(mm_is_user_space(comparison_tmpmap_vaddr)) {
		dbg_printf(__func__, "Temporary mapping area for comparison must be in kernel space, but %p was provided", comparison_tmpmap_vaddr);
		return KM_RESULT_INVALID_ARGS;
	}
	const size_t page_size = mm_get_page_size();
	uint64_t hash_code = kf_djb_hash64((const char *)vaddr, page_size);
	km_result_t result;

	ps::read_semaphore_guard g(ki_ro_pages_cache_lock);

	if (auto it = ki_cached_ro_pages_hash_map.find(hash_code); it != ki_cached_ro_pages_hash_map.end()) {
		for (auto j : it.value()) {
			KM_RETURN_IF_FAILED(mm_mmap(mm_get_cur_context(), comparison_tmpmap_vaddr, j, page_size, MM_PAGE_MAPPED | MM_PAGE_READ, MM_MMAP_NO_PGTAB_ALLOC));

			if(!memcmp(vaddr, comparison_tmpmap_vaddr, page_size)) {
				*paddr_out = j;
				return KM_RESULT_OK;
			}
		}
	}

	*paddr_out = nullptr;
	return KM_RESULT_OK;
}

void ps_ref_cached_ro_page(void *paddr) {
	ps::read_semaphore_guard g(ki_ro_pages_cache_lock);

	if (auto it = ki_cached_ro_pages_paddr_to_hash_map.find(paddr); it != ki_cached_ro_pages_paddr_to_hash_map.end()) {
		auto &tree = ki_cached_ro_pages_hash_map.at(it.value());
		auto registry = tree.find(paddr);
		if (!registry)
			km_panic("Read-only page registry does not present: %p", paddr);

		kf_atomic_inc_size(&static_cast<ki_cached_ro_page_registry *>(registry)->ref_count);
	} else
		km_panic("Unreferencing invalid cached read-only page: %p", paddr);
}

void ps_unref_cached_ro_page(void *paddr) {
	kfxx::scope_guard release_read_lock_guard([]() noexcept {
		ki_ro_pages_cache_lock.read_unlock();
	});
	ki_ro_pages_cache_lock.read_lock();

	if (auto it = ki_cached_ro_pages_paddr_to_hash_map.find(paddr); it != ki_cached_ro_pages_paddr_to_hash_map.end()) {
		auto &tree = ki_cached_ro_pages_hash_map.at(it.value());
		auto registry = tree.find(paddr);
		if (!registry)
			km_panic("Read-only page registry does not present: %p", paddr);

		if (!kf_atomic_dec_size(&static_cast<ki_cached_ro_page_registry *>(registry)->ref_count)) {
			release_read_lock_guard.release();
			ki_ro_pages_cache_lock.read_unlock();

			ps::write_semaphore_guard g(ki_ro_pages_cache_lock);

			tree.remove(registry);
			kfxx::destroy_and_release<ki_cached_ro_page_registry>(&ki_ps_cached_ro_pages_registry_allocator, static_cast<ki_cached_ro_page_registry *>(registry));
			if (!tree.size())
				ki_cached_ro_pages_hash_map.remove(it.value());
		}
	} else
		km_panic("Unreferencing invalid cached read-only page: %p", paddr);
}

km_binproto_t *ps_find_binproto(fs_fcb_t *fcb) {
	// TODO: Add a binproto lock.

	return static_cast<km_binproto_t *>(ki_registered_binprotos.find(fcb));
}

void ps_unregister_binproto(km_binproto_t *proto) {
	// TODO: Add a binproto lock.

	// TODO: Check if the prototype is registered.
	ki_registered_binprotos.remove(proto);
}

km_result_t ps_add_segment_to_binproto(km_binproto_t *proto, void *vaddr_base, size_t size, mm_page_access_t page_access, km_binseg_t **seg_out) {
	const size_t page_size = mm_get_page_size();

	if (vaddr_base != (void *)kfxx::floor_align_to((uintptr_t)vaddr_base, page_size))
		return KM_RESULT_INVALID_ARGS;

	km_binseg_t *seg = (km_binseg_t *)mm_kalloc(sizeof(km_binseg_t), alignof(km_binseg_t));
	kfxx::scope_guard seg_release_guard([seg]() noexcept {
		mm_kfree(seg);
	});

	kfxx::construct_at<km_binseg_t>(seg);

	seg->vaddr_base = vaddr_base;
	seg->size = size;
	seg->access = page_access;
	seg->cur_offset = 0;

	kfxx::scope_guard release_page_pools_guard([seg]() noexcept {
		for (km_binseg_page_pool_t *p = seg->pages, *next; p;) {
			next = p->next;
			for (size_t i = 0; i < p->used_num; ++i) {
				mm_unref_page(p->descs[i].rb_value);
				seg->page_descs_query_tree.remove(&p->descs[i]);
			}
			mm_kfree(p);
		}
	});
	while (seg->cur_offset < size) {
		km_binseg_page_pool_t *page_pool = (km_binseg_page_pool_t *)mm_kalloc(sizeof(km_binseg_page_pool_t), page_size);
		kfxx::scope_guard release_cur_page_pool_guard([seg, page_pool]() noexcept {
			for (size_t i = 0; i < page_pool->used_num; ++i) {
				mm_unref_page(page_pool->descs[i].rb_value);
				seg->page_descs_query_tree.remove(&page_pool->descs[i]);
			}
			mm_kfree(page_pool);
		});

		kfxx::construct_at<km_binseg_page_pool_t>(page_pool);

		for (size_t i = 0; i < PBOS_ARRAYSIZE(page_pool->descs); ++i) {
			kfxx::construct_at<km_binseg_page_desc_t>(&page_pool->descs[i]);

			void *paddr = mm_alloc_single_page(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);

			if (!paddr)
				return KM_RESULT_NO_MEM;

			page_pool->descs[i].rb_value = paddr;
			seg->page_descs_query_tree.insert(&page_pool->descs[i]);

			++page_pool->used_num;
			if ((seg->cur_offset += page_size) >= size)
				break;
		}
		release_cur_page_pool_guard.release();

		if (!seg->cur_page_pool) {
			seg->pages = page_pool;
			seg->cur_page_pool = page_pool;
		} else {
			seg->cur_page_pool->next = page_pool;
			page_pool->prev = seg->cur_page_pool;

			seg->cur_page_pool = page_pool;
		}
	}

	release_page_pools_guard.release();
	seg_release_guard.release();

	*seg_out = seg;

	return KM_RESULT_OK;
}

void *km_ps_paddr_in_binseg(km_binseg_t *seg, void *vaddr) {
	return seg->page_descs_query_tree.find(vaddr);
}

PBOS_EXTERN_C_END
