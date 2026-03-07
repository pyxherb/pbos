#ifndef _FREESTDC_CXX_BASEDEFS_HH_
#define _FREESTDC_CXX_BASEDEFS_HH_

#include <c_basedefs.h>

#if __cplusplus >= 202002L
	#define FREESTDC_CONSTEXPR_SINCE_CXX20 constexpr
#else
	#define FREESTDC_CONSTEXPR_SINCE_CXX20
#endif

#endif
