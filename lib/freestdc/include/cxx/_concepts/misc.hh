#ifndef _FREESTDC_CXX_CONCEPTS_MISC_
#define _FREESTDC_CXX_CONCEPTS_MISC_

#include "ctordtor.hh"
#include "assignable_from.hh"
#include "swappable.hh"
#include "compare.hh"

namespace std {
	template <class T>
	concept movable =
		std::is_object_v<T> &&
		std::move_constructible<T> &&
		std::assignable_from<T &, T> &&
		std::swappable<T>;

	template <class T>
	concept copyable =
		std::copy_constructible<T> &&
		std::movable<T> &&
		std::assignable_from<T &, T &> &&
		std::assignable_from<T &, const T &> &&
		std::assignable_from<T &, const T>;

	template <class T>
	concept semiregular = std::copyable<T> && std::default_initializable<T>;

	template <class T>
	concept regular = std::semiregular<T> && std::equality_comparable<T>;

	template <class F, class... Args>
	concept invocable =
		requires(F &&f, Args &&...args) {
			std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
		};

	template <class F, class... Args>
	concept regular_invocable = std::invocable<F, Args...>;

	template <class F, class... Args>
	concept predicate =
		std::regular_invocable<F, Args...> &&
		boolean_testable<std::invoke_result_t<F, Args...>>;

	template <class R, class T, class U>
	concept relation =
		std::predicate<R, T, T> && std::predicate<R, U, U> &&
		std::predicate<R, T, U> && std::predicate<R, U, T>;

	template <class R, class T, class U>
	concept equivalence_relation = std::relation<R, T, U>;

	template <class R, class T, class U>
	concept strict_weak_order = std::relation<R, T, U>;
}

#endif
