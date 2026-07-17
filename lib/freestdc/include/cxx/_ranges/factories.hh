#ifndef _FREESTDC_CXX_RANGES_FACTORIES_
#define _FREESTDC_CXX_RANGES_FACTORIES_

#include "view.hh"

namespace std {
	namespace ranges {
		template <class T>
			requires std::is_object_v<T>
		class empty_view : public ranges::view_interface<empty_view<T>> {};
	}

	namespace views {
		template <class T>
		constexpr ranges::empty_view<T> empty{};
	}
}

#endif
