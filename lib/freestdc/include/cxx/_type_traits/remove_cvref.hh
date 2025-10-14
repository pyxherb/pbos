#ifndef _FREESTDC_CXX_TYPE_TRAITS_REMOVE_CVREF_
#define _FREESTDC_CXX_TYPE_TRAITS_REMOVE_CVREF_

#include "remove_cv.hh"
#include "remove_reference.hh"

namespace std {
#if __cplusplus >= 202002L
	template <typename T>
	struct remove_cvref {
		using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
	};

	#define __cpp_lib_remove_cvref 201711L
#endif
}

#endif
