#ifndef _PBOS_KFXX_SCOPE_GUARD_HH_
#define _PBOS_KFXX_SCOPE_GUARD_HH_

#include "basedefs.hh"
#include <type_traits>

namespace kfxx {
	template <typename T>
	struct ScopeGuard {
		T callback;
		bool released = false;

		static_assert(std::is_nothrow_invocable_v<T>, "The callback must be noexcept");

		ScopeGuard() = delete;
		PBOS_FORCEINLINE ScopeGuard(T &&callback)
			: callback(std::move(callback)) {
		}
		PBOS_FORCEINLINE ~ScopeGuard() {
			if (!released)
				callback();
		}

		PBOS_FORCEINLINE void release() noexcept {
			released = true;
		}
	};

	template <typename T>
	struct Deferred {
		T callback;
		static_assert(std::is_nothrow_invocable_v<T>, "The callback must be noexcept");

		Deferred() = delete;
		PBOS_FORCEINLINE Deferred(T &&callback)
			: callback(std::move(callback)) {
		}
		PBOS_FORCEINLINE ~Deferred() {
			callback();
		}
	};
}

#endif
