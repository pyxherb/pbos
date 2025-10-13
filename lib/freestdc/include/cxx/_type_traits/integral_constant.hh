#ifndef _FREESTDC_CXX_TYPE_TRAITS_INTEGRAL_CONSTANT_
#define _FREESTDC_CXX_TYPE_TRAITS_INTEGRAL_CONSTANT_

namespace std {
	template <class T, T v>
	struct integral_constant {
		static constexpr T value = v;
		using value_type = T;
		using type = integral_constant;
		constexpr operator value_type() const noexcept { return value; }
		constexpr value_type operator()() const noexcept { return value; }
	};

	template< bool B >
	using bool_constant = integral_constant<bool, B>;

	using true_type = integral_constant<bool, true>;
	using false_type = integral_constant<bool, false>;
}

#define __cpp_lib_integral_constant_callable 201304L
#define __cpp_lib_bool_constant 201505L

#endif
