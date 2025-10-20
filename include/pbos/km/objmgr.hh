#ifndef _KM_OBJMGR_HH_
#define _KM_OBJMGR_HH_

#include "objmgr.h"

namespace om {
	template <typename T>
	PBOS_REQUIRES_CONCEPT()
	class object_ptr final {
	private:
		T *_ptr;
	};
}

#endif
