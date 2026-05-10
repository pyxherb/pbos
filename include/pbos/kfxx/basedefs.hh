#ifndef _PBOS_KFXX_BASEDEFS_H_
#define _PBOS_KFXX_BASEDEFS_H_

#include <pbos/kf/basedefs.h>

#define PBOS_KFXX_API

#ifdef __cplusplus
namespace kfxx {
	enum class iterator_direction : uint8_t {
		Forward = 0,
		Reversed,
		Invalid
	};
}
#endif

#endif
