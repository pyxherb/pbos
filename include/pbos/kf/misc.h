#ifndef _PBOS_KM_MISC_H_
#define _PBOS_KM_MISC_H_

#include "basedefs.h"
#include <string.h>

PBOS_API size_t strlen(const char* str);

PBOS_API char* strcpy(char* dest, const char* src);
PBOS_API char* strncpy(char* dest, const char* src, size_t num);

PBOS_API char* strcat(char* dest, const char* src);
PBOS_API char* strncat(char* dest, const char* src, size_t num);

PBOS_API int strcmp(const char* s1, const char* s2);
PBOS_API int strncmp(const char* s1, const char* s2, size_t num);

PBOS_API char* strchr(const char* str, int c);
PBOS_API char* strrchr(const char* str, int c);

PBOS_API void* memset(void* dest, int c, size_t n);
PBOS_API int memcmp(const void* s1, const void* s2, size_t n);
PBOS_API void* memcpy(void* dest, const void* src, size_t n);
PBOS_API void* memmove(void* dest, const void* src, size_t n);

#endif
