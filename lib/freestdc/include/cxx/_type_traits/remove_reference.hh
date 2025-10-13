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

	template<typename T>
	using remove_reference_t = remove_reference<T>;
}

#endif
