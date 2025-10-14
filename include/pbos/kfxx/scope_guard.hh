#ifndef _PBOS_KFXX_SCOPE_GUARD_HH_
#define _PBOS_KFXX_SCOPE_GUARD_HH_

#include "basedefs.h"

namespace kf {
	template<typename T>
	struct ScopeGuard {
		T callback;
		bool released = false;

		static_assert(std::is_nothrow_invocable_v<T>, "The callback must be noexcept");

		ScopeGuard() = delete;
		PB_FORCEINLINE ScopeGuard(T &&callback)
			: callback(std::move(callback)) {
		}
		PB_FORCEINLINE ~ScopeGuard() {
			if (!released)
				callback();
		}

		PB_FORCEINLINE void release() noexcept {
			released = true;
		}
	};
}

#endif
