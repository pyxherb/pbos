#ifndef _FREESTDC_CXX_TYPE_TRAITS_MAKE_UNSIGNED_
#define _FREESTDC_CXX_TYPE_TRAITS_MAKE_UNSIGNED_

#include <cstdint>
#include "conditional.hh"
#include "is_enum.hh"
#include "underlying_type.hh"

namespace std {
	template <typename T>
	struct _make_unsigned_impl {
	};

	template <>
	struct _make_unsigned_impl<char> {
		typedef unsigned char type;
	};

	template <>
	struct _make_unsigned_impl<short> {
		typedef unsigned short type;
	};

	template <>
	struct _make_unsigned_impl<int> {
		typedef unsigned int type;
	};

	template <>
	struct _make_unsigned_impl<long> {
		typedef unsigned long type;
	};

	template <>
	struct _make_unsigned_impl<long long> {
		typedef unsigned long long type;
	};

	template <>
	struct _make_unsigned_impl<signed char> {
		typedef unsigned char type;
	};

	template <>
	struct _make_unsigned_impl<unsigned char> {
		typedef unsigned char type;
	};

	template <>
	struct _make_unsigned_impl<unsigned short> {
		typedef unsigned short type;
	};

	template <>
	struct _make_unsigned_impl<unsigned int> {
		typedef unsigned int type;
	};

	template <>
	struct _make_unsigned_impl<unsigned long> {
		typedef unsigned long type;
	};

	template <>
	struct _make_unsigned_impl<unsigned long long> {
		typedef unsigned long long type;
	};

	template <>
	struct _make_unsigned_impl<wchar_t> {
		typedef std::conditional<sizeof(wchar_t) <= sizeof(uint8_t),
			uint8_t,
			std::conditional<sizeof(wchar_t) <= sizeof(uint16_t),
				uint16_t,
				std::conditional<sizeof(wchar_t) <= sizeof(uint32_t),
					uint32_t,
					uintmax_t>>::type>::type type;
	};

#if __cplusplus >= 202002L
	template <>
	struct _make_unsigned_impl<char8_t> {
		typedef uint8_t type;
	};
#endif

	template <>
	struct _make_unsigned_impl<char16_t> {
		typedef uint16_t type;
	};

	template <>
	struct _make_unsigned_impl<char32_t> {
		typedef uint32_t type;
	};

	template <typename T>
	struct make_unsigned {
		typedef conditional<std::is_enum<T>::value,
			typename _make_unsigned_impl<typename underlying_type<T>::type>::type::type,
			typename _make_unsigned_impl<T>::type>
			type;
	};

	template <typename T>
	using make_unsigned_t = typename make_unsigned<T>::type;
}

#endif
