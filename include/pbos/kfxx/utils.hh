#ifndef _PBOS_KFXX_UTILS_HH_
#define _PBOS_KFXX_UTILS_HH_

#include "basedefs.hh"
#include <memory>

namespace kf {
#if __cplusplus >= 202002L
	template <typename T, typename... Args>
	requires std::constructible_from<T, Args...>
#else
	template <typename T, typename... Args>
#endif
		PB_FORCEINLINE void constructAt(T *ptr, Args &&...args) {
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
	PB_FORCEINLINE void destroyAt(T *const ptr) {
		std::destroy_at<T>(ptr);
	}
}

#endif
