#include <stdint.h>

uint64_t __mulvdi3(uint64_t x, uint64_t y) {
	uint64_t sum = 0;
	for (uint8_t i = 0; y; ++i) {
		if (y & 1)
			sum += (x << i);
		y >>= 1;
	}
	return sum;
}

uint64_t __udivdi3(uint64_t x, uint64_t y) {
	uint64_t ans = 0;
	for (int8_t i = 63; i >= 0; i--)
		if ((x >> i) >= y) {
			ans = ans | ((uint64_t)1 << i);
			x -= (y << i);
		}
	return ans;
}

uint64_t __umoddi3(uint64_t x, uint64_t y) {
	return x - __mulvdi3(__udivdi3(x, y), y);
}
