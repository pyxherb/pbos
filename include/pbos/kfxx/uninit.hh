#ifndef _PBOS_KFXX_UNINIT_HH_
#define _PBOS_KFXX_UNINIT_HH_

#include "utils.hh"

namespace kfxx {
	template <typename T>
	class Uninit final {
	private:
		alignas(T) char _buf[sizeof(T)];

	public:
		Uninit() noexcept = default;
		PBOS_FORCEINLINE explicit Uninit(T &&data) noexcept {
			construct_at<T>((T *)_buf, std::move(data));
		}
		Uninit(const Uninit<T> &) = delete;
		PBOS_FORCEINLINE ~Uninit() {
		}
		PBOS_FORCEINLINE T &get() {
			return *(T *)_buf;
		}
		PBOS_FORCEINLINE const T &get() const {
			return *(T *)_buf;
		}
		PBOS_FORCEINLINE T &operator*() {
			return *(T *)get();
		}
		PBOS_FORCEINLINE const T &operator*() const {
			return *(T *)get();
		}
		PBOS_FORCEINLINE T *operator->() {
			return (T *)_buf;
		}
		PBOS_FORCEINLINE const T *operator->() const {
			return (T *)_buf;
		}
		PBOS_FORCEINLINE T &&release() {
			return std::move(*(T *)_buf);
		}
		PBOS_FORCEINLINE void destroy() {
			return std::destroy_at<T>((T *)_buf);
		}
		PBOS_FORCEINLINE T *data() {
			return (T *)_buf;
		}
		PBOS_FORCEINLINE const T *data() const {
			return (T *)_buf;
		}
		PBOS_FORCEINLINE void moveFrom(T &&src) {
			construct_at((T *)_buf, std::move(src));
		}
		PBOS_FORCEINLINE Uninit<T> &operator=(T &&rhs) noexcept {
			construct_at((T *)_buf, std::move(rhs));
			return *this;
		}
	};
}

#endif
