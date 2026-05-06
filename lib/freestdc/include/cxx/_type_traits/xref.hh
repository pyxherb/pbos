#ifndef _FREESTDC_CXX_TYPE_TRAITS_XREF_HH_
#define _FREESTDC_CXX_TYPE_TRAITS_XREF_HH_

#include "add_reference.hh"
#include "conditional.hh"
#include "is_lvalue_reference.hh"
#include "is_rvalue_reference.hh"
#include "remove_cv.hh"
#include "remove_reference.hh"

namespace std {
	template <class T>
	struct _xref {
		using type = std::conditional_t<
			std::is_lvalue_reference_v<T>,
			std::add_lvalue_reference_t<std::remove_reference_t<T>>,
			std::conditional_t<std::is_rvalue_reference_v<T>, std::add_rvalue_reference_t<std::remove_reference_t<T>>, std::remove_cv_t<T>>>;
	};

	template <class T>
	using _xref_t = typename _xref<T>::type;
}

#endif
