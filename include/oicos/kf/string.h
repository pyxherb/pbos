#ifndef _OICOS_KF_STRING_H_
#define _OICOS_KF_STRING_H_

#include <string.h>

typedef struct _kf_string_t {
	size_t len;
	char s[];
} kf_string_t;

#endif
