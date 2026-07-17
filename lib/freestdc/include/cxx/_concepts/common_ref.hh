#ifndef _FREESTDC_CXX_CONCEPTS_COMMON_REF_
#define _FREESTDC_CXX_CONCEPTS_COMMON_REF_

#include "same_as.hh"
#include "convertible_to.hh"
#include "../_type_traits/common_reference.hh"

namespace std {
	template <class T, class U>
	concept common_reference_with =
		std::same_as<std::common_reference_t<T, U>, std::common_reference_t<U, T>> &&
		std::convertible_to<T, std::common_reference_t<T, U>> &&
		std::convertible_to<U, std::common_reference_t<T, U>>;
}

#endif
