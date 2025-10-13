#ifndef _FREESTDC_CXX_TYPE_TRAITS_REMOVE_REFERENCE_
#define _FREESTDC_CXX_TYPE_TRAITS_REMOVE_REFERENCE_

namespace std {
	template <class T>
	struct remove_reference {
		typedef T type;
	};
	template <class T>
	struct remove_reference<T &> {
		typedef T type;
	};
	template <class T>
	struct remove_reference<T &&> {
		typedef T type;
	};
}

#endif
