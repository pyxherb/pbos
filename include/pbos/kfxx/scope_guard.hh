#ifndef _PBOS_KFXX_SCOPE_GUARD_HH_
#define _PBOS_KFXX_SCOPE_GUARD_HH_

#include "basedefs.hh"

namespace kfxx {
	template<typename T>
	struct scope_guard {
		T callback;
		bool released = false;

		static_assert(std::is_nothrow_invocable_v<T>, "The callback must be noexcept");

		scope_guard() = delete;
		PBOS_FORCEINLINE scope_guard(T &&callback)
			: callback(std::move(callback)) {
		}
		PBOS_FORCEINLINE ~scope_guard() {
			if (!released)
				callback();
		}

		PBOS_FORCEINLINE void release() noexcept {
			released = true;
		}
	};
}

#endif
