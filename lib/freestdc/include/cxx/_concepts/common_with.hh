#ifndef _FREESTDC_CXX_CONCEPTS_COMMON_WITH_
#define _FREESTDC_CXX_CONCEPTS_COMMON_WITH_

#include "common_ref.hh"

namespace std {
	template <class T, class U>
	concept common_with =
		std::same_as<std::common_type_t<T, U>, std::common_type_t<U, T>> &&
		requires {
			static_cast<std::common_type_t<T, U>>(std::declval<T>());
			static_cast<std::common_type_t<T, U>>(std::declval<U>());
		} &&
		std::common_reference_with<
			std::add_lvalue_reference_t<const T>,
			std::add_lvalue_reference_t<const U>> &&
		std::common_reference_with<
			std::add_lvalue_reference_t<std::common_type_t<T, U>>,
			std::common_reference_t<
				std::add_lvalue_reference_t<const T>,
				std::add_lvalue_reference_t<const U>>>;
}

#endif
