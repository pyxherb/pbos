#ifndef _PBOS_KI_EXEC_HH_
#define _PBOS_KI_EXEC_HH_

#include <pbos/generated/km.h>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/kfxx/uuid.hh>
#include <pbos/ps/exec.h>
#include <pbos/kfxx/allocator.hh>
#include <pbos/ki/mm/context.hh>

PBOS_EXTERN_C_BEGIN

class ki_ps_cached_ro_pages_buckets_allocator_t : public kfxx::Alloc {
public:
	PBOS_PRIVATE ki_ps_cached_ro_pages_buckets_allocator_t();
	PBOS_PRIVATE virtual ~ki_ps_cached_ro_pages_buckets_allocator_t();

	PBOS_PRIVATE virtual size_t inc_ref() noexcept override;
	PBOS_PRIVATE virtual size_t dec_ref() noexcept override;

	PBOS_PRIVATE virtual void *alloc(size_t size, size_t alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
	PBOS_PRIVATE virtual void release(void *ptr, size_t size, size_t alignment) noexcept override;

	PBOS_PRIVATE virtual void *type_identity() const noexcept override;
};

class ki_ps_cached_ro_pages_registry_allocator_t : public kfxx::Alloc {
public:
	PBOS_PRIVATE ki_ps_cached_ro_pages_registry_allocator_t();
	PBOS_PRIVATE virtual ~ki_ps_cached_ro_pages_registry_allocator_t();

	PBOS_PRIVATE virtual size_t inc_ref() noexcept override;
	PBOS_PRIVATE virtual size_t dec_ref() noexcept override;

	PBOS_PRIVATE virtual void *alloc(size_t size, size_t alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
	PBOS_PRIVATE virtual void release(void *ptr, size_t size, size_t alignment) noexcept override;

	PBOS_PRIVATE virtual void *type_identity() const noexcept override;
};

typedef struct _ki_binldr_registry_t : public kfxx::RBTree<kf_uuid_t>::Node {
	km_binldr_ops_t ops;
} ki_binldr_registry_t;

typedef struct _km_binseg_page_desc_t : public kfxx::RBTree<void *>::Node {
} km_binseg_page_desc_t;

typedef struct alignas(PAGESIZE) _km_binseg_page_pool_t {
	_km_binseg_page_pool_t *prev = nullptr, *next = nullptr;
	size_t used_num = 0;
	km_binseg_page_desc_t descs[(PAGESIZE - sizeof(void *) - sizeof(void *) - sizeof(size_t)) / sizeof(km_binseg_page_desc_t)];
} km_binseg_page_pool_t;

static_assert(sizeof(km_binseg_page_pool_t) == PAGESIZE);

typedef struct _km_binseg_t {
	_km_binseg_t *prev, *next;
	void *vaddr_base;
	size_t size;
	size_t cur_offset;
	km_binseg_page_pool_t *pages = nullptr, *cur_page_pool = nullptr;
	kfxx::RBTree<void *> page_descs_query_tree;
	mm_pgaccess_t access;
} km_binseg_t;

typedef struct _km_binproto_t : public kfxx::RBTree<fs_fcb_t *>::Node {
	km_binseg_t *segments = nullptr;
} km_binproto_t;

extern kfxx::RBTree<kf_uuid_t> ki_registered_binldrs;

extern km_init_binldr_registry_t ki_builtin_binldrs[];

extern kfxx::RBTree<fs_fcb_t *> ki_registered_binprotos;

void ki_load_init();
void ki_init_binldrs();

PBOS_EXTERN_C_END

#endif
