#include <pbos/kf/hash.h>

uint64_t kf_hash_djb(const char *src, size_t size) {
	uint64_t hash = 5381;
	for (size_t i = 0; i < size; ++i) {
		hash += (hash << 5) + *(src++);
	}
	return hash;
}
