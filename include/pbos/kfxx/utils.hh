#ifndef _PBOS_KFXX_UTILS_HH_
#define _PBOS_KFXX_UTILS_HH_

#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include "basedefs.hh"

namespace kfxx {
	template <typename T, typename... Args>
	// PBOS_REQUIRES_CONCEPT(std::constructible_from<T, Args...>)
	PBOS_FORCEINLINE void construct_at(T *ptr, Args &&...args) {
#ifdef new
	#if __cplusplus >= 202002L
		std::construct_at<T>(ptr, std::forward<Args>(args)...);
	#else
		#if defined(_MSC_VER) || (defined(__GNUC__)) || (defined(__clang__))
			#pragma push_macro("new")
			#undef new
		new (ptr) T(std::forward<Args>(args)...);
			#pragma pop_macro("new")
		#else
		std::allocator_traits<std::allocator<T>> allocator;
		allocator.construct(ptr, std::forward<Args>(args)...);
		#endif
	#endif
#else
		new (ptr) T(std::forward<Args>(args)...);
#endif
	}

	template <typename T>
	PBOS_FORCEINLINE void destroy_at(T *const ptr) {
		std::destroy_at<T>(ptr);
	}

	template <typename T>
	PBOS_FORCEINLINE void move_assign_or_move_construct(T &lhs, T &&rhs) noexcept {
		if constexpr (std::is_move_assignable_v<T>) {
			lhs = std::move(rhs);
		} else {
			static_assert(std::is_move_constructible_v<T>, "The type must at least be move-constructible");
			std::destroy_at<T>(&lhs);
			construct_at<T>(&lhs, std::move(lhs));
		}
	}

	template <typename T>
	PBOS_FORCEINLINE constexpr T ceil_align_to(T data, T alignment) {
		static_assert(std::is_unsigned_v<T>, "ceil_align_to only applies to unsigned types");
		kd_assert(alignment);
		return data + ((alignment - data % alignment) % alignment);
	}

	template <typename T, T alignment>
	PBOS_FORCEINLINE constexpr T ceil_align_to(T data) {
		static_assert(std::is_unsigned_v<T>, "ceil_align_to only applies to unsigned types");
		static_assert(alignment != 0, "alignment must not be 0");
		if constexpr ((alignment & (alignment - 1)) == 0) {
			return (data + (alignment - 1)) & ~(alignment - 1);
		}
		return data + ((alignment - data % alignment) % alignment);
	}

	template <typename T>
	PBOS_FORCEINLINE constexpr size_t ceil_align_to_type(size_t data) {
		return (data + (alignof(T) - 1)) & ~(alignof(T) - 1);
	}

	template <typename T>
	PBOS_FORCEINLINE constexpr T floor_align_to(T data, T alignment) {
		static_assert(std::is_unsigned_v<T>, "floor_align_to only applies to unsigned types");
		kd_assert(alignment);
		return data - data % alignment;
	}

	template <typename T, T alignment>
	PBOS_FORCEINLINE constexpr T floor_align_to(T data) {
		static_assert(std::is_unsigned_v<T>, "floor_align_to only applies to unsigned types");
		static_assert(alignment != 0, "alignment must not be 0");
		if constexpr ((alignment & (alignment - 1)) == 0) {
			return data & ~(alignment - 1);
		}
		return data - data % alignment;
	}

	template <typename T>
	PBOS_FORCEINLINE constexpr size_t floor_align_to_type(size_t data) {
		return data & ~(alignof(T) - 1);
	}
}

#endif
