#ifndef _FREESTDC_STRING_H_
#define _FREESTDC_STRING_H_

#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char* str);

char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t num);

char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, size_t num);

int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t num);

char* strchr(const char* str, int c);
char* strrchr(const char* str, int c);

void* memset(void* dest, int c, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
void* memcpy(void* dest, const void* src, size_t n);

#ifdef __cplusplus
}
#endif

#endif
