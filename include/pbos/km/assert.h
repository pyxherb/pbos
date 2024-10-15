#ifndef _KM_ASSERT_H_
#define _KM_ASSERT_H_

#include <pbos/km/panic.h>

#ifndef _NDEBUG
	#undef assert
	#define assert(c)         \
		if (!(c)) km_panic(      \
			"Assertion error!\n" \
			"File: %s\n"         \
			"Line: %d",          \
			__FILE__, __LINE__)

	#undef kd_dbgcheck
	#define kd_dbgcheck(c, fmt, ...) \
		if (!(c)) km_panic(fmt, ##__VA_ARGS__)
#else
	#define assert(c)
	#define kd_dbgcheck(c, msg)
#endif

#endif
