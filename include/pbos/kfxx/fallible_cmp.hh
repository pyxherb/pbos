#ifndef _PBOS_KFXX_FALLIBLE_CMP_H_
#define _PBOS_KFXX_FALLIBLE_CMP_H_

#include "basedefs.hh"
#include "option.hh"

namespace kfxx {
	template <typename T>
	struct fallible_less {
		option_t<bool> operator()(const T &lhs, const T &rhs) const {
			return lhs < rhs;
		}
	};

	template <typename T>
	struct fallible_greater {
		option_t<bool> operator()(const T &lhs, const T &rhs) const {
			return lhs > rhs;
		}
	};

	template <typename T>
	struct fallible_equal_to {
		option_t<bool> operator()(const T &lhs, const T &rhs) const {
			return lhs == rhs;
		}
	};
}

#endif
