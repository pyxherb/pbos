#ifndef _FREESTDC_CXX_TYPE_TRAITS_DECAY_
#define _FREESTDC_CXX_TYPE_TRAITS_DECAY_

#include "remove_reference.hh"
#include "conditional.hh"
#include "is_array.hh"
#include "add_pointer.hh"
#include "remove_extent.hh"
#include "is_function.hh"
#include "remove_cv.hh"

namespace std {
	template <typename T>
	struct decay {
	private:
		typedef typename std::remove_reference<T>::type U;

	public:
		typedef typename std::conditional<
			std::is_array<U>::value,
			typename std::add_pointer<typename std::remove_extent<U>::type>::type,
			typename std::conditional<
				std::is_function<U>::value,
				typename std::add_pointer<U>::type,
				typename std::remove_cv<U>::type>::type>::type type;
	};
}

#endif
