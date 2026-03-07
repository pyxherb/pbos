#ifndef _FREESTDC_CXX_TYPE_TRAITS_MAKE_SIGNED_
#define _FREESTDC_CXX_TYPE_TRAITS_MAKE_SIGNED_

#include <cstdint>
#include "conditional.hh"
#include "is_enum.hh"
#include "underlying_type.hh"

namespace std {
	template <typename T>
	struct _make_signed_impl {
	};

	template <>
	struct _make_signed_impl<char> {
		typedef signed char type;
	};

	template <>
	struct _make_signed_impl<short> {
		typedef signed short type;
	};

	template <>
	struct _make_signed_impl<int> {
		typedef signed int type;
	};

	template <>
	struct _make_signed_impl<long> {
		typedef signed long type;
	};

	template <>
	struct _make_signed_impl<long long> {
		typedef signed long long type;
	};

	template <>
	struct _make_signed_impl<signed char> {
		typedef signed char type;
	};

	template <>
	struct _make_signed_impl<unsigned char> {
		typedef signed char type;
	};

	template <>
	struct _make_signed_impl<unsigned short> {
		typedef signed short type;
	};

	template <>
	struct _make_signed_impl<unsigned int> {
		typedef signed int type;
	};

	template <>
	struct _make_signed_impl<unsigned long> {
		typedef signed long type;
	};

	template <>
	struct _make_signed_impl<unsigned long long> {
		typedef signed long long type;
	};

	template <>
	struct _make_signed_impl<wchar_t> {
		typedef std::conditional<sizeof(wchar_t) <= sizeof(uint8_t),
			int8_t,
			std::conditional<sizeof(wchar_t) <= sizeof(uint16_t),
				int16_t,
				std::conditional<sizeof(wchar_t) <= sizeof(uint32_t),
					int32_t,
					intmax_t>>::type>::type type;
	};

#if __cplusplus >= 202002L
	template <>
	struct _make_signed_impl<char8_t> {
		typedef int8_t type;
	};
#endif

	template <>
	struct _make_signed_impl<char16_t> {
		typedef int16_t type;
	};

	template <>
	struct _make_signed_impl<char32_t> {
		typedef int32_t type;
	};

	template <typename T>
	struct make_signed {
		typedef conditional<std::is_enum<T>::value,
			typename _make_signed_impl<typename underlying_type<T>::type>::type::type,
			typename _make_signed_impl<T>::type>
			type;
	};

	template <typename T>
	using make_signed_t = typename make_signed<T>::type;
}

#endif
