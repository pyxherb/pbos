#ifndef _PBOS_KF_BASEDEFS_H_
#define _PBOS_KF_BASEDEFS_H_

#include <pbos/common.h>

#if PBOS_COMPILING_KERNEL
	#define PBOS_API PBOS_EXPORTED
#else
	#define PBOS_API
#endif
#define PBOS_PRIVATE

#endif
