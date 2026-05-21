#ifndef _PBOS_KF_BASEDEFS_H_
#define _PBOS_KF_BASEDEFS_H_

#include <pbos/common.h>

#if PBOS_COMPILING_KERNEL
	#define PBOS_API PBOS_EXPORTED
#else
	#define PBOS_API PBOS_IMPORTED
#endif
#define PBOS_PRIVATE

#define PBOS_KMOD_API PBOS_EXPORTED
#define PBOS_KMOD_PRIVATE

#endif
