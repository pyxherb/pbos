#ifndef _KM_OBJMGR_HH_
#define _KM_OBJMGR_HH_

#include "objmgr.h"

namespace om {
	template <typename T>
	class object_ptr final {
	private:
		using this_t = object_ptr<T>;

		T *_ptr;

		PBOS_FORCEINLINE void _reset() {
			if (_ptr)
				om_decref(static_cast<om_object_t*>(_ptr));
		}

		PBOS_FORCEINLINE void _set(T *ptr) {
			if (ptr)
				om_incref(static_cast<om_object_t*>(ptr));
			_ptr = ptr;
		}

	public:
		PBOS_FORCEINLINE object_ptr(): _ptr(nullptr) {
		}

		PBOS_FORCEINLINE object_ptr(T *ptr) {
			_set(ptr);
		}

		PBOS_FORCEINLINE object_ptr(const this_t &ptr) {
			_set(ptr._ptr);
		}

		PBOS_FORCEINLINE object_ptr(this_t &&ptr) {
			_set(ptr._ptr);
			ptr._ptr = nullptr;
		}

		PBOS_FORCEINLINE object_ptr &operator=(const this_t &ptr) {
			_reset();
			_set(ptr._ptr);
			return *this;
		}

		PBOS_FORCEINLINE object_ptr &operator=(this_t &&ptr) {
			_reset();
			_set(ptr._ptr);
			ptr._ptr = nullptr;
			return *this;
		}

		PBOS_FORCEINLINE T *get() const {
			return _ptr;
		}

		PBOS_FORCEINLINE T **get_addr() {
			_reset();
			return &_ptr;
		}

		PBOS_FORCEINLINE T **get_addr_no_release() const {
			return &_ptr;
		}

		PBOS_FORCEINLINE T *operator->() const {
			return _ptr;
		}

		PBOS_FORCEINLINE void reset() {
			_reset();
		}

		PBOS_FORCEINLINE T *release() {
			T *ptr = _ptr;
			_ptr = nullptr;
			return ptr;
		}
	};
}

#endif
