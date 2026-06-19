#ifndef _PBOS_KI_KM_KIMA_H_
#define _PBOS_KI_KM_KIMA_H_

#include <pbos/kfxx/allocator.hh>
#include <pbos/ps/mutex.hh>
#include "misc.hh"

PBOS_EXTERN_C_BEGIN

#define KIMA_SMALL_BLOCK_MIN_ORDER 3
#define KIMA_SMALL_BLOCK_MAX_ORDER 9

#define KIMA_ORDERED_BLOCK_SIZE(ord) (1 << (ord))

typedef struct _kima_ublk_poolpg_t kima_ublk_poolpg_t;
typedef struct _kima_vpgdesc_poolpg_t kima_vpgdesc_poolpg_t;

struct kima_small_block_page_desc_t {
	kima_small_block_page_desc_t *prev = nullptr, *next = nullptr;
	size_t num_used = 0;
	size_t order = 0;
};

struct kima_small_block_desc_t {
	kima_small_block_desc_t *prev = nullptr, *next = nullptr;
	void *ptr = nullptr;
	size_t allocated_size;
	bool is_free = true;
};

// +-----------------------------------+--------------------+-----------+
// |            block data             |     block desc     | page desc |
// +-----------------------------------+--------------------+-----------+
struct kima_small_block_page_info_t {
	size_t max_block_capacity;
	size_t block_descs_off;
	size_t page_descs_off;
};

typedef struct _kima_pool_t {
	ps::mutex_t mutex;

	kima_ublk_poolpg_t *ublk_poolpg_list = nullptr;
	kfxx::rbtree_t<void *> ublk_query_tree, ublk_free_tree;

	kima_vpgdesc_poolpg_t *vpgdesc_poolpg_list = nullptr;
	kfxx::rbtree_t<void *> vpgdesc_query_tree, vpgdesc_free_tree;

	kima_small_block_page_desc_t *small_block_pages = nullptr;
	kima_small_block_page_info_t small_block_page_info[KIMA_SMALL_BLOCK_MAX_ORDER - KIMA_SMALL_BLOCK_MIN_ORDER + 1];
	kima_small_block_desc_t
		*free_small_block_descs[KIMA_SMALL_BLOCK_MAX_ORDER - KIMA_SMALL_BLOCK_MIN_ORDER + 1],
		*used_small_block_descs[KIMA_SMALL_BLOCK_MAX_ORDER - KIMA_SMALL_BLOCK_MIN_ORDER + 1];

	size_t num_allocated_pages = 0;

	size_t page_size = 0;

	bool _initialized = false;

	size_t ublk_slots_off, ublk_slots_size;

	size_t vpgdesc_slots_off, vpgdesc_slots_size;

	_kima_pool_t();
	_kima_pool_t(_kima_pool_t &&rhs);
	~_kima_pool_t();
} kima_pool_t;

extern kima_pool_t *mm_global_pool;

void kima_init_pool(kima_pool_t *pool);

//
// User block definitions.
//

typedef struct _kima_ublk_t : public kfxx::rbtree_t<void *>::node_t {
	size_t size;
} kima_ublk_t;

typedef struct _kima_ublk_poolpg_t kima_ublk_poolpg_t;

typedef struct _kima_ublk_poolpg_header_t {
	kima_ublk_poolpg_t *prev, *next;
	size_t used_num;
} kima_ublk_poolpg_header_t;

typedef struct _kima_ublk_poolpg_t {
	kima_ublk_poolpg_header_t header;
} kima_ublk_poolpg_t;

kima_ublk_t *kima_lookup_ublk(kima_pool_t *pool, void *ptr);
kima_ublk_t *kima_lookup_nearest_ublk(kima_pool_t *pool, void *ptr);
kima_ublk_t *kima_alloc_ublk(kima_pool_t *pool, void *ptr, size_t size);
void kima_free_ublk(kima_pool_t *pool, kima_ublk_t *ublk);

//
// Virtual page descriptor definitions.
//

typedef struct _kima_vpgdesc_t : public kfxx::rbtree_t<void *>::node_t {
	size_t ref_count = 0;
	size_t recommended_alloc_off = 0;
} kima_vpgdesc_t;

typedef struct _kima_vpgdesc_poolpg_t kima_vpgdesc_poolpg_t;

typedef struct _kima_vpgdesc_poolpg_header_t {
	kima_vpgdesc_poolpg_t *prev, *next;
	size_t used_num;
} kima_vpgdesc_poolpg_header_t;

typedef struct _kima_vpgdesc_poolpg_t {
	kima_vpgdesc_poolpg_header_t header;
} kima_vpgdesc_poolpg_t;

PBOS_FORCEINLINE size_t kima_log2(size_t size) {
	size_t result = 0;
	while (size)
		size >>= 1, ++result;
	return result;
}

size_t kima_calc_small_block_index(kima_pool_t *pool, size_t order, size_t off);
void kima_calc_small_block_page_info(kima_pool_t *pool, size_t order, kima_small_block_page_info_t *info_out);
size_t kima_calc_small_block_capacity(kima_pool_t *pool, size_t order);
void *kima_alloc_small_block_page(kima_pool_t *pool, size_t order);
void kima_free_small_block_desc(kima_pool_t *pool, kima_small_block_page_desc_t *page_desc, kima_small_block_desc_t *desc);
kima_small_block_desc_t *kima_alloc_small_block_desc(kima_pool_t *pool, size_t order);

kima_vpgdesc_t *kima_lookup_vpgdesc(kima_pool_t *pool, void *ptr);
kima_vpgdesc_t *kima_alloc_vpgdesc(kima_pool_t *pool, void *ptr);
void kima_free_vpgdesc(kima_pool_t *pool, kima_vpgdesc_t *vpgdesc);

void *kima_vpgalloc(kima_pool_t *pool, size_t size);
void kima_vpgfree(kima_pool_t *pool, void *addr, size_t size);

//
// Common allocation APIs.
//

PBOS_NODISCARD void *kima_alloc(kima_pool_t *pool, size_t size, size_t alignment);
PBOS_NODISCARD void *kima_realloc(kima_pool_t *pool, void *old_ptr, size_t size, size_t alignment);
PBOS_NODISCARD void *kima_realloc_in_place(kima_pool_t *pool, void *old_ptr, size_t size);
void kima_free(kima_pool_t *pool, void *ptr);

void kima_free_pool(kima_pool_t *pool);

size_t kima_get_allocated_page_num(kima_pool_t *pool);

class kima_allocator_t : public kfxx::allocator_t {
public:
	kima_pool_t pool;

	PBOS_PRIVATE kima_allocator_t();
	PBOS_PRIVATE virtual ~kima_allocator_t();

	PBOS_PRIVATE virtual size_t inc_ref() noexcept override;
	PBOS_PRIVATE virtual size_t dec_ref() noexcept override;

	PBOS_PRIVATE virtual void *alloc(size_t size, size_t alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size) noexcept override;
	PBOS_PRIVATE virtual void release(void *ptr, size_t size, size_t alignment) noexcept override;

	PBOS_PRIVATE virtual void *type_identity() const noexcept override;
};

PBOS_EXTERN_C_END

#endif
