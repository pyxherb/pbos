#include <stdint.h>
#include <string.h>
#include <emmintrin.h>

void *memset(void *dest, int c, size_t n) {
	c &= 0xff;
	if (!(n & 0b111)) {
		uint64_t _c = c;
		_c = (_c << 56) |
			 (_c << 48) |
			 (_c << 40) |
			 (_c << 32) |
			 (_c << 24) |
			 (_c << 16) |
			 (_c << 8) |
			 _c;
		n >>= 3;
		for (size_t i = 0; i < n; ++i) {
			((uint64_t *)dest)[i] = (uint64_t)_c;
		}
	} else if (!(n & 0b11)) {
		uint32_t _c = c;
		_c = (_c << 24) |
			 (_c << 16) |
			 (_c << 8) |
			 _c;
		n >>= 2;
		for (size_t i = 0; i < n; ++i) {
			((uint32_t *)dest)[i] = (uint32_t)c;
		}
	} else if (!(n & 0b1)) {
		uint16_t _c = c;
		_c = (_c << 8) |
			 _c;
		n >>= 1;
		for (size_t i = 0; i < n; ++i) {
			((uint16_t *)dest)[i] = (uint16_t)c;
		}
	} else {
		for (size_t i = 0; i < n; ++i) {
			((uint8_t *)dest)[i] = (uint8_t)c;
		}
	}
	return dest;
}
