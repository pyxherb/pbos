#ifndef _FREESTDC_CXX_CONCEPTS_BOOLEAN_TESTABLE_
#define _FREESTDC_CXX_CONCEPTS_BOOLEAN_TESTABLE_

#include "convertible_to.hh"

namespace std {
	template <class B>
	concept _boolean_testable_impl = std::convertible_to<B, bool>;

	template <class B>
	concept boolean_testable =
		_boolean_testable_impl<B> &&
		requires(B &&b) {
			{ !std::forward<B>(b) } -> _boolean_testable_impl;
		};
}

#endif
