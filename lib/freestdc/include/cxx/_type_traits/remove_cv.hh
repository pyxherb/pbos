#ifndef _FREESTDC_CXX_TYPE_TRAITS_REMOVE_CV_
#define _FREESTDC_CXX_TYPE_TRAITS_REMOVE_CV_

namespace std {
	template <typename T>
	struct remove_cv {
		typedef T type;
	};
	template <typename T>
	struct remove_cv<const T> {
		typedef T type;
	};
	template <typename T>
	struct remove_cv<volatile T> {
		typedef T type;
	};
	template <typename T>
	struct remove_cv<const volatile T> {
		typedef T type;
	};

	template <typename T>
	struct remove_const {
		typedef T type;
	};
	template <typename T>
	struct remove_const<const T> {
		typedef T type;
	};

	template <typename T>
	struct remove_volatile {
		typedef T type;
	};
	template <typename T>
	struct remove_volatile<volatile T> {
		typedef T type;
	};
}

#endif
