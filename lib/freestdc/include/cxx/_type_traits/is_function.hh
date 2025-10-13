#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_FUNCTION_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_FUNCTION_

#include "integral_constant.hh"

namespace std {
	template <class>
	struct is_function : std::false_type {};

	template <class Ret, class... Args>
	struct is_function<Ret(Args...)> : std::true_type {};

	template <class Ret, class... Args>
	struct is_function<Ret(Args......)> : std::true_type {};

	template <class Ret, class... Args>
	struct is_function<Ret(Args...) const> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) volatile> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) const volatile> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) const> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) volatile> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) const volatile> : std::true_type {};

	template <class Ret, class... Args>
	struct is_function<Ret(Args...) &> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) const &> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) volatile &> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) const volatile &> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) &> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) const &> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) volatile &> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) const volatile &> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) &&> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) const &&> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) volatile &&> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) const volatile &&> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) &&> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) const &&> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) volatile &&> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) const volatile &&> : std::true_type {};

#if __cplusplus >= 201703L
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) const noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) volatile noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) const volatile noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) const noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) volatile noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) const volatile noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) & noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) const & noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) volatile & noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) const volatile & noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) & noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) const & noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) volatile & noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) const volatile & noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) && noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) const && noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) volatile && noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args...) const volatile && noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) && noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) const && noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) volatile && noexcept> : std::true_type {};
	template <class Ret, class... Args>
	struct is_function<Ret(Args......) const volatile && noexcept> : std::true_type {};
#endif
}

#endif
