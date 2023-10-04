#ifndef _FREESTDC_STDIO_H_
#define _FREESTDC_STDIO_H_

#include <stdarg.h>
#include "stddef.h"

int sprintf(char* s, const char *fmt, ...);
int vsprintf(char* s, const char *fmt, va_list args);

#endif
