#ifndef _HAL_IRQ_HH_
#define _HAL_IRQ_HH_

#include <pbos/km/assert.h>
#include <pbos/kfxx/basedefs.hh>
#include "irq.h"

namespace io {
	class irq_disable_lock final {
	private:
		bool _int_disabled : 1;

	public:
		PBOS_FORCEINLINE irq_disable_lock() noexcept {
			if (!(irq_is_disabled())) {
				irq_disable();
				_int_disabled = true;
			} else
				_int_disabled = false;
		}

		PBOS_FORCEINLINE ~irq_disable_lock() {
			if (_int_disabled)
				irq_enable();
		}
	};
}

#endif
