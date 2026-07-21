#ifndef _PBOS_KFXX_SCOPE_GUARD_HH_
#define _PBOS_KFXX_SCOPE_GUARD_HH_

#include <type_traits>
#include "basedefs.hh"

namespace kfxx {
	template <typename T>
	struct scope_guard {
		T callback;
		bool released = false;

		static_assert(std::is_nothrow_invocable_v<T>, "The callback must be noexcept");

		scope_guard() = delete;
		PBOS_FORCEINLINE scope_guard(T &&callback)
			: callback(std::forward<T &&>(callback)) {
		}
		PBOS_FORCEINLINE ~scope_guard() {
			if (!released)
				callback();
		}

		PBOS_FORCEINLINE void release() noexcept {
			released = true;
		}
	};

	template <typename T>
	struct switchable_scope_guard {
		T callback;
		bool enabled;

		static_assert(std::is_nothrow_invocable_v<T>, "The callback must be noexcept");

		switchable_scope_guard() = delete;
		PBOS_FORCEINLINE switchable_scope_guard(T &&callback, bool enabled = true)
			: callback(std::forward<T &&>(callback)), enabled(enabled) {
		}
		PBOS_FORCEINLINE ~switchable_scope_guard() {
			if (enabled)
				callback();
		}

		PBOS_FORCEINLINE void enable() noexcept {
			enabled = true;
		}

		PBOS_FORCEINLINE void disable() noexcept {
			enabled = false;
		}
	};

	template <typename T>
	struct deferred {
		T callback;
		static_assert(std::is_nothrow_invocable_v<T>, "The callback must be noexcept");

		deferred() = delete;
		PBOS_FORCEINLINE deferred(T &&callback)
			: callback(std::move(callback)) {
		}
		PBOS_FORCEINLINE ~deferred() {
			callback();
		}
	};
}

#endif
