#ifndef _FREESTDC_CXX_TYPE_TRAITS_ADD_POINTER_
#define _FREESTDC_CXX_TYPE_TRAITS_ADD_POINTER_

#include "remove_reference.hh"
#include "void_t.hh"

namespace std {
	template <typename T>
	struct _add_pointer_type_identity {
		using type = T;
	};

	template <typename T>
	auto _try_add_pointer(int)
		-> _add_pointer_type_identity<typename std::remove_reference<T>::type *>;

	template <typename T>
	auto _try_add_pointer(...)
		-> _add_pointer_type_identity<T>;

	template <typename T>
	struct add_pointer : decltype(_try_add_pointer<T>(0)) {};
}

#endif
