#ifndef _FREESTDC_CXX_TYPE_TRAITS_ADD_CV_
#define _FREESTDC_CXX_TYPE_TRAITS_ADD_CV_

namespace std {
	template <class T>
	struct add_cv {
		typedef const volatile T type;
	};

	template <class T>
	struct add_const {
		typedef const T type;
	};

	template <class T>
	struct add_volatile {
		typedef volatile T type;
	};
}

#endif
