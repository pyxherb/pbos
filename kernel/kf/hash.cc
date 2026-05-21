#include <pbos/kf/hash.h>
#include <pbos/ki/km/symbol.hh>

PBOS_API uint32_t kf_djb_hash32(const char *src, size_t size) {
	uint32_t hash = 5381;
	for (size_t i = 0; i < size; ++i) {
		hash += (hash << 5) + *(src++);
	}
	return hash;
}

PBOS_API uint64_t kf_djb_hash64(const char *src, size_t size) {
	uint64_t hash = 5381;
	for (size_t i = 0; i < size; ++i) {
		hash += (hash << 5) + *(src++);
	}
	return hash;
}
