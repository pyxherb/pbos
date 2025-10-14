#ifndef _PBOS_KFXX_BASEDEFS_H_
#define _PBOS_KFXX_BASEDEFS_H_

#include <pbos/kf/basedefs.h>

#define PB_KFXX_API

namespace kf {
	enum class IteratorDirection : bool {
		Forward = 0,
		Backward
	};
}

#endif
