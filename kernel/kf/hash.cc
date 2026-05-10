#include <pbos/kf/hash.h>
#include <pbos/ki/km/symbol.hh>

uint32_t kf_djb_hash32(const char *src, size_t size) {
	uint32_t hash = 5381;
	for (size_t i = 0; i < size; ++i) {
		hash += (hash << 5) + *(src++);
	}
	return hash;
}
KI_EXPORT_IMAGE_SYMBOL(kf_djb_hash32);

uint64_t kf_djb_hash64(const char *src, size_t size) {
	uint64_t hash = 5381;
	for (size_t i = 0; i < size; ++i) {
		hash += (hash << 5) + *(src++);
	}
	return hash;
}
KI_EXPORT_IMAGE_SYMBOL(kf_djb_hash64);
