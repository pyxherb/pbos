#include <pbos/kf/hashmap.h>
#include <pbos/km/assert.h>
#include <pbos/km/mm.h>
#include <stdlib.h>
#include <string.h>

static km_result_t _kf_hashmap_resize_buckets(kf_hashmap_t *dest, size_t size);
static km_result_t _kf_hashmap_insert(kf_hashmap_t *dest, kf_hashmap_node_t *node);

void kf_hashmap_init(
	kf_hashmap_t *dest,
	kf_hashmap_hasher_t hasher,
	kf_hashmap_nodefree_t node_free,
	kf_hashmap_nodecmp_t node_cmp,
	kf_hashmap_keycmp_t key_cmp) {
	dest->node_num = 0;
	dest->bucket_num = 0;
	dest->buckets = NULL;

	dest->hasher = hasher;
	dest->node_free = node_free;
	dest->node_cmp = node_cmp;
	dest->key_cmp = key_cmp;

	_kf_hashmap_resize_buckets(dest, KF_HASHMAP_BUCKET_MIN);
}

km_result_t kf_hashmap_insert(kf_hashmap_t *dest, kf_hashmap_node_t *node) {
	{
		km_result_t result = KM_RESULT_OK;

		if (dest->node_num >= (dest->bucket_num << 1))
			result = _kf_hashmap_resize_buckets(dest, dest->bucket_num << 1);
		// else if ((dest->node_num < (dest->bucket_num >> 1)) && (dest->bucket_num > KF_HASHMAP_BUCKET_MIN))
		else if ((dest->node_num < (dest->bucket_num >> 1)) && (dest->bucket_num > (KF_HASHMAP_BUCKET_MIN << 1)))
			result = _kf_hashmap_resize_buckets(dest, dest->bucket_num >> 1);

		if (KM_FAILED(result))
			return result;
	}

	return _kf_hashmap_insert(dest, node);
}

km_result_t kf_hashmap_remove(kf_hashmap_t *dest, kf_hashmap_node_t *node) {
	if ((!dest) || (!node) || (!node->owner_bucket) || (node->owner_bucket->owner_map != dest))
		return KM_MAKEERROR(KM_RESULT_INVALID_ARGS);

	if (node->owner_bucket->nodes == node) {
		kf_list_node_t *next = kf_list_next(&node->list_header);
		node->owner_bucket->nodes = next ? PB_CONTAINER_OF(kf_hashmap_node_t, list_header, next) : NULL;
	}
	kf_list_remove(&node->list_header);

	dest->node_free(node);

	--dest->node_num;

	{
		km_result_t result = KM_RESULT_OK;

		if (dest->node_num >= (dest->bucket_num << 1))
			result = _kf_hashmap_resize_buckets(dest, dest->bucket_num << 1);
		else if ((dest->node_num < (dest->bucket_num >> 1)) && (dest->bucket_num > (KF_HASHMAP_BUCKET_MIN << 1)))
			result = _kf_hashmap_resize_buckets(dest, dest->bucket_num >> 1);

		if (KM_FAILED(result))
			return result;
	}

	return KM_RESULT_OK;
}

void kf_hashmap_free(kf_hashmap_t *dest) {
	for (size_t i = 0; i < dest->bucket_num; ++i) {
		kf_hashmap_bucket_t *bucket = &dest->buckets[i];

		if (bucket->nodes) {
			kf_list_foreach(i, &bucket->nodes[0].list_header) {
				dest->node_free(PB_CONTAINER_OF(kf_hashmap_node_t, list_header, i));
			}
		}
	}

	mm_kfree(dest->buckets);
}

kf_hashmap_node_t *kf_hashmap_find(kf_hashmap_t *dest, const void *key) {
	size_t index = dest->hasher(dest->bucket_num, key, true);
	if (index >= dest->bucket_num)
		return NULL;

	kf_list_foreach(i, &dest->buckets[index].nodes->list_header) {
		if (dest->key_cmp(PB_CONTAINER_OF(kf_hashmap_node_t, list_header, i), key))
			return PB_CONTAINER_OF(kf_hashmap_node_t, list_header, i);
	}

	return NULL;
}

kf_hashmap_node_t *kf_hashmap_begin(kf_hashmap_t *dest) {
	for (size_t i = 0; i < dest->bucket_num; ++i) {
		if (dest->buckets[i].nodes)
			return dest->buckets[i].nodes;
	}
	return NULL;
}

kf_hashmap_node_t *kf_hashmap_end(kf_hashmap_t *dest) {
	for (size_t i = dest->bucket_num; i; --i) {
		if (dest->buckets[i - 1].nodes)
			return PB_CONTAINER_OF(kf_hashmap_node_t, list_header, kf_list_end(&dest->buckets[i - 1].nodes->list_header));
	}
	return NULL;
}

kf_hashmap_node_t *kf_hashmap_next(kf_hashmap_node_t *cur) {
	kf_hashmap_t *owner_map = cur->owner_bucket->owner_map;
	kf_list_node_t *next = kf_list_next(&cur->list_header);
	if (next)
		return PB_CONTAINER_OF(kf_hashmap_node_t, list_header, next);

	for (size_t i = cur->owner_bucket - owner_map->buckets + 1; i < owner_map->bucket_num; ++i)
		if (owner_map->buckets[i].nodes)
			return owner_map->buckets[i].nodes;
	return NULL;
}

static km_result_t _kf_hashmap_insert(kf_hashmap_t *dest, kf_hashmap_node_t *node) {
	size_t index = dest->hasher(dest->bucket_num, node, false);
	if (index >= dest->bucket_num)
		return KM_MAKEERROR(KM_RESULT_INTERNAL_ERROR);

	kf_hashmap_bucket_t *bucket = &(dest->buckets[index]);

	memset(&(node->list_header), 0, sizeof(node->list_header));

	node->owner_bucket = bucket;
	if (bucket->nodes) {
		bucket->nodes->list_header.last = &node->list_header;
		// kf_list_append(&bucket->nodes->list_header, &node->list_header);
	}
	// else
		bucket->nodes = node;

	++dest->node_num;

	return KM_RESULT_OK;
}

#include <pbos/km/logger.h>

static km_result_t _kf_hashmap_resize_buckets(kf_hashmap_t *dest, size_t size) {
	kprintf("resizing buckets, size = %d\n", (int)size);
	assert(size >= KF_HASHMAP_BUCKET_MIN);
	kf_hashmap_bucket_t *new_buckets = mm_kmalloc(sizeof(kf_hashmap_bucket_t) * size);

	if (!new_buckets)
		return KM_MAKEERROR(KM_RESULT_NO_MEM);

	memset(new_buckets, 0, sizeof(kf_hashmap_bucket_t) * size);

	for (size_t i = 0; i < size; ++i) {
		new_buckets[i].owner_map = dest;
		new_buckets[i].nodes = NULL;
	}

	if (dest->buckets) {
		// Save the old buckets pointer and the size - we will need them to rollback when errors occurred.
		size_t old_bucket_num = dest->bucket_num;
		kf_hashmap_bucket_t *old_buckets = dest->buckets;

		dest->bucket_num = size;
		dest->buckets = new_buckets;

		// Move the nodes into new buckets.
		for (size_t i = 0; i < old_bucket_num; ++i) {
			kf_hashmap_bucket_t *bucket = old_buckets + i;
			for (kf_list_node_t *i = &bucket->nodes->list_header; i;) {
				km_result_t result;

				kf_list_node_t *next = i->next;

				result = _kf_hashmap_insert(dest, PB_CONTAINER_OF(kf_hashmap_node_t, list_header, i));
				assert(KM_SUCCEEDED(result));

				i = next;
			}
		}
	} else {
		dest->bucket_num = size;
		dest->buckets = new_buckets;
	}

	return KM_RESULT_OK;
}
