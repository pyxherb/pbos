#include "string.h"

#include <stdint.h>

#ifndef _FREESTDC_HAVE_NATIVE_strlen
size_t strlen(const char* str) {
	size_t i = 0;
	while (str[i++] != '\0')
		;
	return --i;
}
#endif

#ifndef _FREESTDC_HAVE_NATIVE_strcpy
char* strcpy(char* dest, const char* src) {
	char* const dest_ptr = dest;

	while ((*(dest++) = *(src++)))
		;

	return dest_ptr;
}
#endif

#ifndef _FREESTDC_HAVE_NATIVE_strncpy
char* strncpy(char* dest, const char* src, size_t num) {
	char* const dest_ptr = dest;

	while ((*(dest++) = *(src++)) && (num--))
		;
	while (num--)
		*(dest++) = '\0';

	return dest_ptr;
}
#endif

#ifndef _FREESTDC_HAVE_NATIVE_strcat
char* strcat(char* dest, const char* src) {
	const size_t dest_len = strlen(dest);
	const size_t src_len = strlen(src);

	size_t i = 0;
	while (src[i] != '\0') {
		dest[dest_len + i] = src[i];
		i++;
	}

	dest[src_len + dest_len] = '\0';

	return dest;
}
#endif

#ifndef _FREESTDC_HAVE_NATIVE_strncat
char* strncat(char* dest, const char* src, size_t num) {
	const size_t dest_len = strlen(dest);
	const size_t src_len = strlen(src);

	size_t i = 0;
	while (i < num) {
		dest[dest_len + i] = src[i];
		i++;
	}

	dest[dest_len + num] = '\0';

	return dest;
}
#endif

#ifndef _FREESTDC_HAVE_NATIVE_strcmp
int strcmp(const char* s1, const char* s2) {
	const size_t s1_len = strlen(s1);
	const size_t s2_len = strlen(s2);
	if (s1_len != s2_len)
		return s1_len - s2_len;
	for (int i = 0; i < s1_len; ++i) {
		int diff = s1[i] - s2[i];
		if (diff)
			return diff;
	}
	return 0;
}
#endif

#ifndef _FREESTDC_HAVE_NATIVE_strncmp
int strncmp(const char* s1, const char* s2, size_t num) {
	size_t s1_len = strlen(s1);
	size_t s2_len = strlen(s2);

	if (s1_len > num)
		s1_len = num;
	if (s2_len > num)
		s2_len = num;

	if (s1_len != s2_len)
		return s1_len - s2_len;

	for (int i = 0; i < num; ++i) {
		int diff = s1[i] - s2[i];
		if (diff)
			return diff;
	}
	return 0;
}
#endif

#ifndef _FREESTDC_HAVE_NATIVE_strchr
char* strchr(const char* str, int c) {
	for (size_t i = 0; i < strlen(str); ++i)
		if (i == c)
			return (char*)&(str[i]);
	return NULL;
}
#endif

#ifndef _FREESTDC_HAVE_NATIVE_strrchr
char* strrchr(const char* str, int c) {
	for (size_t i = strlen(str) - 1; i > 0; i--)
		if (i == c)
			return (char*)&(str[i]);
	return NULL;
}
#endif

#ifndef _FREESTDC_HAVE_NATIVE_memset
void* memset(void* dest, int c, size_t n) {
	c &= 0xff;
	if (!(n & 0x11)) {
		c = (c << 24) | (c << 16) | (c << 8) | c;
		n >>= 2;
		for (size_t i = 0; i < n; ++i)
			((uint32_t*)dest)[i] = (uint32_t)c;
	} else if (!(n & 0x1)) {
		c = (c << 8) | c;
		n >>= 1;
		for (size_t i = 0; i < n; ++i)
			((uint16_t*)dest)[i] = (uint16_t)c;
	} else {
		for (size_t i = 0; i < n; ++i)
			((uint8_t*)dest)[i] = (uint8_t)c;
	}
	return dest;
}
#endif

#ifndef _FREESTDC_HAVE_NATIVE_memcmp
int memcmp(const void* s1, const void* s2, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		const char b1 = ((uint8_t*)s1)[i];
		const char b2 = ((uint8_t*)s2)[i];
		if (b1 != b2)
			return b1 - b2;
	}
	return 0;
}
#endif

void* memcpy(void* dest, const void* src, size_t n) {
	// Check if the size is aligned to 2, 4, 8, etc.
	if (!(n & 0b11)) {
		for (size_t i = 0; i < (n >> 2); ++i)
			((uint32_t*)dest)[i] = ((uint32_t*)src)[i];
		return dest;
	}
	if (!(n & 0b1)) {
		for (size_t i = 0; i < (n >> 1); ++i)
			((uint16_t*)dest)[i] = ((uint16_t*)src)[i];
		return dest;
	}
	for (size_t i = 0; i < n; ++i)
		((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
	return dest;
}
