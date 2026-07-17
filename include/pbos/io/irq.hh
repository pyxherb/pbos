#ifndef _HAL_IRQ_HH_
#define _HAL_IRQ_HH_

#include "irq.h"

namespace io {
	class local_irq_lock final {
	private:
		bool _int_disabled : 1;

	public:
		PBOS_FORCEINLINE local_irq_lock() noexcept {
			if (!(io_is_irq_disabled())) {
				io_disable_irq();
				_int_disabled = true;
			} else
				_int_disabled = false;
		}

		PBOS_FORCEINLINE ~local_irq_lock() {
			if (_int_disabled)
				io_enable_irq();
		}
	};
}

#endif
