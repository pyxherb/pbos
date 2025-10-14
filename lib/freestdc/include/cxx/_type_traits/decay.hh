#ifndef _FREESTDC_CXX_TYPE_TRAITS_DECAY_
#define _FREESTDC_CXX_TYPE_TRAITS_DECAY_

#include "add_pointer.hh"
#include "conditional.hh"
#include "is_array.hh"
#include "is_function.hh"
#include "remove_cv.hh"
#include "remove_extent.hh"
#include "remove_reference.hh"

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

	template <typename T>
	using decay_t = typename decay<T>::type;
}

#endif
