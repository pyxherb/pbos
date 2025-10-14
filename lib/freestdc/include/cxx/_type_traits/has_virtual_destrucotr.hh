#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_CONSTRUCTIBLE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_CONSTRUCTIBLE_

#include <utility>
#include "add_cv.hh"
#include "integral_constant.hh"
#include "remove_extent.hh"
#include "void_t.hh"

namespace std {
	template <typename T>
	struct has_virtual_destructor : bool_constant<__has_virtual_destructor(T)> {};
}

#endif
