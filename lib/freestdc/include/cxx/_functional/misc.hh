#ifndef _FREESTDC_CXX_FUNCTIONAL_MISC_
#define _FREESTDC_CXX_FUNCTIONAL_MISC_

#include <_type_traits/is_invocable.hh>
#include <_type_traits/remove_cvref.hh>
#include <_type_traits/decay.hh>
#include <_utility/forward.hh>
#include <cxx_basedefs.hh>
#include <memory>

namespace std {
	struct identity {
		template <typename T>
		FREESTDC_FORCEINLINE constexpr T &&operator()(T &&t) const noexcept;

		using is_transparent = int;
	};

	namespace detail {
		template <typename T>
		constexpr T &f(T &t) noexcept { return t; }
		template <typename T>
		void f(T &&) = delete;
	}

	template <typename T>
	class reference_wrapper {
	public:
		using type = T;

		template <class U, class = decltype(detail::f<T>(std::declval<U>()), std::enable_if_t<!std::is_same_v<reference_wrapper, std::remove_cvref_t<U>>>())>
		constexpr reference_wrapper(U &&u) noexcept(noexcept(detail::f<T>(std::forward<U>(u))))
			: _ptr(std::addressof(detail::f<T>(std::forward<U>(u)))) {}

		reference_wrapper(const reference_wrapper &) noexcept = default;

		reference_wrapper &operator=(const reference_wrapper &x) noexcept = default;

		constexpr operator T &() const noexcept { return *_ptr; }
		constexpr T &get() const noexcept { return *_ptr; }

		template <class... ArgTypes>
		constexpr std::invoke_result_t<T &, ArgTypes...>
		operator()(ArgTypes &&...args) const
			noexcept(std::is_nothrow_invocable_v<T &, ArgTypes...>) {
			return std::invoke(get(), std::forward<ArgTypes>(args)...);
		}

	private:
		T *_ptr;
	};

	template <typename T>
	reference_wrapper(T &) -> reference_wrapper<T>;

	template <typename T>
	struct unwrap_reference {
		using type = T;
	};
	template <class U>
	struct unwrap_reference<std::reference_wrapper<U>> {
		using type = U &;
	};

	template <typename T>
	using unwrap_reference_t = typename unwrap_reference<T>::type;

	template <typename T>
	struct unwrap_ref_decay : std::unwrap_reference<std::decay_t<T>> {};

	template <typename T>
	using unwrap_ref_decay_t = typename unwrap_ref_decay<T>::type;

#define __cpp_lib_unwrap_ref 201811L
}

#endif
