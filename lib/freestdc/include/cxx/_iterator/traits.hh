#ifndef _FREESTDC_CXX_ITERATOR_TRAITS_
#define _FREESTDC_CXX_ITERATOR_TRAITS_

#include <type_traits>
#include "iter_tags.hh"

namespace std {
	template <class It>
	struct iterator_traits {
		using value_type = typename It::value_type;
		using difference_type = typename It::difference_type;
		using pointer = typename It::pointer;
		using reference = typename It::reference;
		using iterator_category = typename It::iterator_category;
	};

	template <class T>
	struct iterator_traits<T *> {
		using value_type = std::remove_cv_t<T>;
		using difference_type = std::ptrdiff_t;
		using pointer = T *;
		using reference = T &;
		using iterator_category = std::random_access_iterator_tag;
	};

	template <class>
	struct incrementable_traits {};

	template <class T>
	struct incrementable_traits<T *> {
		using difference_type = std::ptrdiff_t;
	};

	template <class T>
		requires requires { typename T::difference_type; }
	struct incrementable_traits<T> {
		using difference_type = typename T::difference_type;
	};

	template <class T>
		requires(!requires { typename T::difference_type; } && requires { typename iterator_traits<T>::difference_type; })
	struct incrementable_traits<T> {
		using difference_type = typename iterator_traits<T>::difference_type;
	};

	template <class>
	struct indirectly_readable_traits {};

	template <class T>
	struct indirectly_readable_traits<T *> {
		using value_type = std::remove_cv_t<T>;
	};

	template <class T, std::size_t N>
	struct indirectly_readable_traits<T[N]> {
		using value_type = std::remove_cv_t<T>;
	};

	template <class T>
		requires requires { typename T::value_type; }
	struct indirectly_readable_traits<T> {
		using value_type = typename T::value_type;
	};

	template <class T>
		requires(!requires { typename T::value_type; } && requires { typename iterator_traits<T>::value_type; })
	struct indirectly_readable_traits<T> {
		using value_type = typename iterator_traits<T>::value_type;
	};
}

#endif
