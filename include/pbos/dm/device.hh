#ifndef _PBOS_DM_DEVICE_HH_
#define _PBOS_DM_DEVICE_HH_

#include <pbos/kfxx/rcobj.hh>
#include "device.h"

namespace dm {
	struct device_inc_ref {
		PBOS_FORCEINLINE void operator()(dm_device_t *ptr) noexcept {
			dm_ref_device(ptr);
		}
	};

	struct device_dec_ref {
		PBOS_FORCEINLINE void operator()(dm_device_t *ptr) noexcept {
			dm_unref_device(ptr);
		}
	};

	using device_ptr = kfxx::custom_rc_object_ptr<dm_device_t, device_inc_ref, device_dec_ref>;
}

#endif
