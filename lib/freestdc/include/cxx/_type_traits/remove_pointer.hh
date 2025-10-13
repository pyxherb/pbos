#ifndef _FREESTDC_CXX_TYPE_TRAITS_REMOVE_POINTER_
#define _FREESTDC_CXX_TYPE_TRAITS_REMOVE_POINTER_

namespace std {
	template <class T>
	struct remove_pointer {
		typedef T type;
	};
	template <class T>
	struct remove_pointer<T *> {
		typedef T type;
	};
	template <class T>
	struct remove_pointer<T *const> {
		typedef T type;
	};
	template <class T>
	struct remove_pointer<T *volatile> {
		typedef T type;
	};
	template <class T>
	struct remove_pointer<T *const volatile> {
		typedef T type;
	};
}

#endif
