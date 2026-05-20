#ifndef _PBOS_KFXX_UNIQUE_PTR_HH_
#define _PBOS_KFXX_UNIQUE_PTR_HH_

#include <memory>
#include <type_traits>
#include <utility>
#include "allocator.hh"
#include "basedefs.hh"
#include "rcobj.hh"

namespace kfxx {
	template <typename T>
	struct kernel_allocator_stated_deleter {
		PBOS_FORCEINLINE void operator()(T *ptr) noexcept {
			kfxx::destroy_and_release<T>(kfxx::kernel_allocator(), ptr);
		}
	};

	template <typename T>
	struct allocator_stated_deleter {
	private:
		rc_object_ptr<allocator_t> _allocator;

	public:
		PBOS_FORCEINLINE allocator_stated_deleter(allocator_t *allocator) noexcept : _allocator(allocator) {}
		PBOS_FORCEINLINE ~allocator_stated_deleter() {}
		PBOS_FORCEINLINE void operator()(T *ptr) noexcept {
			kfxx::destroy_and_release<T>(_allocator, ptr);
		}
		PBOS_FORCEINLINE void set_allocator(allocator_t *ptr) noexcept {
			_allocator = ptr;
		}
	};

	template <typename T, typename D = kernel_allocator_stated_deleter<T>>
	struct unique_ptr {
	private:
		static_assert(std::is_invocable_v<D, T *>, "The deleter is not invocable");
		T *_ptr;
		[[no_unique_address]] D _deleter;

		using ThisType = kfxx::unique_ptr<T, D>;

	public:
		PBOS_FORCEINLINE void reset() noexcept {
			if (_ptr) {
				_deleter(_ptr);
				_ptr = nullptr;
			}
		}

		PBOS_FORCEINLINE unique_ptr(T *ptr, D deleter = {}) noexcept : _ptr(ptr), _deleter(std::move(deleter)) {
		}
		unique_ptr(const ThisType &other) noexcept = delete;
		PBOS_FORCEINLINE unique_ptr(ThisType &&other) noexcept : _ptr(other._ptr), _deleter(other._deleter) {
			other._ptr = nullptr;
		}
		PBOS_FORCEINLINE ~unique_ptr() {
			reset();
		}

		PBOS_FORCEINLINE ThisType &operator=(T *ptr) noexcept {
			reset();
			_ptr = ptr;
			return *this;
		}
		ThisType &operator=(const ThisType &other) = delete;
		PBOS_FORCEINLINE ThisType &operator=(ThisType &&other) noexcept {
			reset();

			_deleter = std::move(other._deleter);
			_ptr = other._ptr;
			other._ptr = nullptr;

			return *this;
		}

		PBOS_FORCEINLINE T *get() const noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE T *&get_ref() noexcept {
			reset();
			return _ptr;
		}
		PBOS_FORCEINLINE T *&get_ref_without_release() noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE T *const &get_Ref_without_release() const noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE T **get_addr() noexcept {
			reset();
			return &_ptr;
		}
		PBOS_FORCEINLINE T **get_addr_without_release() noexcept {
			return &_ptr;
		}
		PBOS_FORCEINLINE T *const *get_addr_without_release() const noexcept {
			return &_ptr;
		}
		PBOS_FORCEINLINE T *release() noexcept {
			T *ptr = _ptr;
			_ptr = nullptr;
			return ptr;
		}
		PBOS_FORCEINLINE T *operator->() const noexcept {
			return _ptr;
		}

		PBOS_FORCEINLINE bool operator<(const T *rhs) const noexcept {
			return _ptr < rhs;
		}

		PBOS_FORCEINLINE bool operator>(const T *rhs) const noexcept {
			return _ptr > rhs;
		}

		PBOS_FORCEINLINE bool operator==(const T *rhs) const noexcept {
			return _ptr == rhs;
		}

		PBOS_FORCEINLINE bool operator!=(const T *rhs) const noexcept {
			return _ptr != rhs;
		}

		PBOS_FORCEINLINE bool operator<(const ThisType &rhs) const noexcept {
			return _ptr < rhs._ptr;
		}

		PBOS_FORCEINLINE bool operator>(const ThisType &rhs) const noexcept {
			return _ptr > rhs._ptr;
		}

		PBOS_FORCEINLINE operator bool() const noexcept {
			return _ptr;
		}

		PBOS_FORCEINLINE bool operator==(const ThisType &rhs) const noexcept {
			return _ptr == rhs._ptr;
		}

		PBOS_FORCEINLINE bool operator!=(const ThisType &rhs) const noexcept {
			return _ptr != rhs._ptr;
		}

		PBOS_FORCEINLINE const D &get_deleter() const noexcept {
			return _deleter;
		}

		PBOS_FORCEINLINE D &get_deleter() noexcept {
			return _deleter;
		}
	};
}

#endif
