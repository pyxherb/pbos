#include <pbos/kf/misc.h>
#include <kernel/generated/config.hh>

#ifndef _PBOS_HAVE_NATIVE_strlen
PBOS_API size_t strlen(const char *str) {
	size_t i = 0;
	while (str[i++] != '\0');
	return --i;
}
#endif

#ifndef _PBOS_HAVE_NATIVE_strcpy
PBOS_API char *strcpy(char *dest, const char *src) {
	char *const dest_ptr = dest;

	while ((*(dest++) = *(src++)));

	return dest_ptr;
}
#endif

#ifndef _PBOS_HAVE_NATIVE_strncpy
PBOS_API char *strncpy(char *dest, const char *src, size_t num) {
	char *const dest_ptr = dest;

	while ((*(dest++) = *(src++)) && (num--));
	while (num--)
		*(dest++) = '\0';

	return dest_ptr;
}
#endif

#ifndef _PBOS_HAVE_NATIVE_strcat
PBOS_API char *strcat(char *dest, const char *src) {
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

#ifndef _PBOS_HAVE_NATIVE_strncat
PBOS_API char *strncat(char *dest, const char *src, size_t num) {
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

#ifndef _PBOS_HAVE_NATIVE_strcmp
PBOS_API int strcmp(const char *s1, const char *s2) {
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

#ifndef _PBOS_HAVE_NATIVE_strncmp
PBOS_API int strncmp(const char *s1, const char *s2, size_t num) {
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

#ifndef _PBOS_HAVE_NATIVE_strchr
PBOS_API char *strchr(const char *str, int c) {
	for (size_t i = 0; i < strlen(str); ++i)
		if (i == c)
			return (char *)&(str[i]);
	return NULL;
}
#endif

#ifndef _PBOS_HAVE_NATIVE_strrchr
PBOS_API char *strrchr(const char *str, int c) {
	for (size_t i = strlen(str) - 1; i > 0; i--)
		if (i == c)
			return (char *)&(str[i]);
	return NULL;
}
#endif

#ifndef _PBOS_HAVE_NATIVE_memset
PBOS_API void *memset(void *dest, int c, size_t n) {
	c &= 0xff;
	if ((!(n & 0b111)) && !(((uintptr_t)dest) & 0b111)) {
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
	} else if ((!(n & 0b11) && !(((uintptr_t)dest) & 0b11))) {
		uint32_t _c = c;
		_c = (_c << 24) |
			 (_c << 16) |
			 (_c << 8) |
			 _c;
		n >>= 2;
		for (size_t i = 0; i < n; ++i) {
			((uint32_t *)dest)[i] = (uint32_t)_c;
		}
	} else if ((!(n & 0b1) && !(((uintptr_t)dest) & 0b1))) {
		uint16_t _c = c;
		_c = (_c << 8) |
			 _c;
		n >>= 1;
		for (size_t i = 0; i < n; ++i) {
			((uint16_t *)dest)[i] = (uint16_t)_c;
		}
	} else {
		for (size_t i = 0; i < n; ++i) {
			((uint8_t *)dest)[i] = (uint8_t)c;
		}
	}
	return dest;
}
#endif

PBOS_NO_ASAN void *ki_raw_memset(void *dest, int c, size_t n) {
#ifdef _PBOS_HAVE_NATIVE_memset
	memset(dest, c, n);
#else
	c &= 0xff;
	if ((!(n & 0b111)) && (!(((uintptr_t)dest) & 0b111))) {
		uint64_t _c = c;
		_c = (_c << 56) |
			 (_c << 48) |
			 (_c << 40) |
			 (_c << 32) |
			 (_c << 24) |
			 (_c << 16) |
			 (_c << 8) |
			 _c;
		n /= sizeof(uint64_t);
		for (size_t i = 0; i < n; ++i) {
			((uint64_t *)dest)[i] = (uint64_t)_c;
		}
	} else if ((!(n & 0b11)) && !(((uintptr_t)dest) & 0b11)) {
		uint32_t _c = c;
		_c = (_c << 24) |
			 (_c << 16) |
			 (_c << 8) |
			 _c;
		n /= sizeof(uint32_t);
		for (size_t i = 0; i < n; ++i) {
			((uint32_t *)dest)[i] = (uint32_t)_c;
		}
	} else if ((!(n & 0b1)) && (!(((uintptr_t)dest) & 0b1))) {
		uint16_t _c = c;
		_c = (_c << 8) |
			 _c;
		n /= sizeof(uint16_t);
		for (size_t i = 0; i < n; ++i) {
			((uint16_t *)dest)[i] = (uint16_t)_c;
		}
	} else
	{
		for (size_t i = 0; i < n; ++i) {
			((uint8_t *)dest)[i] = (uint8_t)c;
		}
	}
	return dest;
#endif
}

#ifndef _PBOS_HAVE_NATIVE_memcmp
PBOS_API int memcmp(const void *s1, const void *s2, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		const char b1 = ((uint8_t *)s1)[i];
		const char b2 = ((uint8_t *)s2)[i];
		if (b1 != b2)
			return b1 - b2;
	}
	return 0;
}
#endif

PBOS_API void *memcpy(void *dest, const void *src, size_t n) {
	// Check if the size is aligned to 2, 4, 8, etc.
	if ((!(n & 0b111)) && !(((uintptr_t)dest) & 0b111)) {
		n >>= 3;
		for (size_t i = 0; i < n; ++i)
			((uint64_t *)dest)[i] = ((uint64_t *)src)[i];
		return dest;
	}
	if ((!(n & 0b11)) && !(((uintptr_t)dest) & 0b11)) {
		n >>= 2;
		for (size_t i = 0; i < n; ++i)
			((uint32_t *)dest)[i] = ((uint32_t *)src)[i];
		return dest;
	}
	if ((!(n & 0b1)) && !(((uintptr_t)dest) & 0b1)) {
		n >>= 1;
		for (size_t i = 0; i < n; ++i)
			((uint16_t *)dest)[i] = ((uint16_t *)src)[i];
		return dest;
	}
	for (size_t i = 0; i < n; ++i)
		((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
	return dest;
}

PBOS_NO_ASAN void *ki_raw_memcpy(void *dest, const void *src, size_t n) {
	if ((!(n & 0b111)) && !(((uintptr_t)dest) & 0b111)) {
		n >>= 3;
		for (size_t i = 0; i < n; ++i)
			((uint64_t *)dest)[i] = ((uint64_t *)src)[i];
		return dest;
	}
	if ((!(n & 0b11)) && !(((uintptr_t)dest) & 0b11)) {
		n >>= 2;
		for (size_t i = 0; i < n; ++i)
			((uint32_t *)dest)[i] = ((uint32_t *)src)[i];
		return dest;
	}
	if ((!(n & 0b1)) && !(((uintptr_t)dest) & 0b1)) {
		n >>= 1;
		for (size_t i = 0; i < n; ++i)
			((uint16_t *)dest)[i] = ((uint16_t *)src)[i];
		return dest;
	}
	for (size_t i = 0; i < n; ++i)
		((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
	return dest;
}

PBOS_API void *memmove(void *dest, const void *src, size_t n) {
	if (((const char *)src + n) < (const char *)dest)
		return memcpy(dest, src, n);
	// Check if the size is aligned to 2, 4, 8, etc.
	if ((!(n & 0b111)) && !(((uintptr_t)dest) & 0b111)) {
		n >>= 3;
		for (size_t i = n; i; --i)
			((uint64_t *)dest)[i - 1] = ((uint64_t *)src)[i - 1];
		return dest;
	}
	if ((!(n & 0b11)) && !(((uintptr_t)dest) & 0b11)) {
		n >>= 2;
		for (size_t i = n; i; --i)
			((uint32_t *)dest)[i - 1] = ((uint32_t *)src)[i - 1];
		return dest;
	}
	if ((!(n & 0b1)) && !(((uintptr_t)dest) & 0b1)) {
		n >>= 1;
		for (size_t i = n; i; --i)
			((uint16_t *)dest)[i - 1] = ((uint16_t *)src)[i - 1];
		return dest;
	}
	for (size_t i = n; i; --i)
		((uint8_t *)dest)[i - 1] = ((uint8_t *)src)[i - 1];
	return dest;
}

PBOS_NO_ASAN void *ki_raw_memmove(void *dest, const void *src, size_t n) {
#ifdef _PBOS_HAVE_NATIVE_memmove
	memmove(dest, c, n);
#else
	if (((const char *)src + n) < (const char *)dest)
		return memcpy(dest, src, n);
	// Check if the size is aligned to 2, 4, 8, etc.
	if ((!(n & 0b111)) && !(((uintptr_t)dest) & 0b111)) {
		n >>= 3;
		for (size_t i = n; i; --i)
			((uint64_t *)dest)[i - 1] = ((uint64_t *)src)[i - 1];
		return dest;
	}
	if ((!(n & 0b11)) && !(((uintptr_t)dest) & 0b11)) {
		n >>= 2;
		for (size_t i = n; i; --i)
			((uint32_t *)dest)[i - 1] = ((uint32_t *)src)[i - 1];
		return dest;
	}
	if ((!(n & 0b1)) && !(((uintptr_t)dest) & 0b1)) {
		n >>= 1;
		for (size_t i = n; i; --i)
			((uint16_t *)dest)[i - 1] = ((uint16_t *)src)[i - 1];
		return dest;
	}
	for (size_t i = n; i; --i)
		((uint8_t *)dest)[i - 1] = ((uint8_t *)src)[i - 1];
	return dest;
#endif
}
