#ifndef _PBOS_KF_HASHMAP_H_
#define _PBOS_KF_HASHMAP_H_

#include "list.h"
#include <pbos/utils.h>
#include <pbos/km/result.h>
#include <stdbool.h>

#define KF_HASHMAP_BUCKET_MIN 4

typedef struct _kf_hashmap_t kf_hashmap_t;
typedef struct _kf_hashmap_bucket_t kf_hashmap_bucket_t;

typedef struct _kf_hashmap_node_t {
	kf_list_node_t list_header;
	kf_hashmap_bucket_t *owner_bucket;
} kf_hashmap_node_t;

typedef struct _kf_hashmap_bucket_t {
	kf_hashmap_node_t* nodes;
	kf_hashmap_t *owner_map;
} kf_hashmap_bucket_t;

typedef size_t (*kf_hashmap_hasher_t)(size_t bucket_num, const void* target, bool is_target_key);
typedef void (*kf_hashmap_nodefree_t)(kf_hashmap_node_t *node);
typedef bool (*kf_hashmap_nodecmp_t)(const kf_hashmap_node_t *lhs, const kf_hashmap_node_t *rhs);
typedef bool (*kf_hashmap_keycmp_t)(const kf_hashmap_node_t *lhs, const void *key);

typedef struct _kf_hashmap_t {
	kf_hashmap_bucket_t *buckets;
	size_t bucket_num;
	size_t node_num;

	kf_hashmap_hasher_t hasher;
	kf_hashmap_nodefree_t node_free;
	kf_hashmap_nodecmp_t node_cmp;
	kf_hashmap_keycmp_t key_cmp;
} kf_hashmap_t;

void kf_hashmap_init(
	kf_hashmap_t *dest,
	kf_hashmap_hasher_t hasher,
	kf_hashmap_nodefree_t node_free,
	kf_hashmap_nodecmp_t node_cmp,
	kf_hashmap_keycmp_t key_cmp);

km_result_t kf_hashmap_insert(kf_hashmap_t *dest, kf_hashmap_node_t *node);
km_result_t kf_hashmap_remove(kf_hashmap_t *dest, kf_hashmap_node_t *node);
void kf_hashmap_free(kf_hashmap_t *dest);

kf_hashmap_node_t *kf_hashmap_find(kf_hashmap_t *dest, const void *key);

kf_hashmap_node_t *kf_hashmap_begin(kf_hashmap_t *dest);
kf_hashmap_node_t *kf_hashmap_end(kf_hashmap_t *dest);
kf_hashmap_node_t *kf_hashmap_next(kf_hashmap_node_t *cur);

#define kf_hashmap_size(dest) (dest->node_num)

#endif
