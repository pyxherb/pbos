#ifndef _PBOS_PS_MUTEX_HH_
#define _PBOS_PS_MUTEX_HH_

#include "mutex.h"
#include <pbos/km/assert.h>

namespace ps {
	class mutex_t final {
	private:
		ps_mutex_t _mutex;

	public:
		PBOS_FORCEINLINE mutex_t() noexcept {
			ps_init_mutex(&_mutex);
		}

		PBOS_FORCEINLINE ~mutex_t() {
			kd_assert(_mutex._data.spinlock == HAL_SPINLOCK_UNLOCKED);
		}

		PBOS_FORCEINLINE void lock() noexcept {
			ps_lock_mutex(&_mutex);
		}

		PBOS_FORCEINLINE void unlock() noexcept {
			ps_unlock_mutex(&_mutex);
		}

		PBOS_FORCEINLINE bool try_lock() noexcept {
			return ps_try_lock_mutex(&_mutex);
		}
	};
}

#endif
