#ifndef _PBOS_KF_BASEDEFS_H_
#define _PBOS_KF_BASEDEFS_H_

#include <pbos/common.h>

#define PB_KF_API

#ifdef __cplusplus
namespace kf {
	enum class IteratorDirection : bool {
		Forward = 0,
		Backward
	}
}
#endif

#endif
