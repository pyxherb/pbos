#ifndef _PBOS_KFXX_FALLIBLE_CMP_H_
#define _PBOS_KFXX_FALLIBLE_CMP_H_

#include <functional>
#include "cmp.hh"
#include "basedefs.hh"
#include "option.hh"

namespace kfxx {
	template <typename T>
	struct FallibleLt {
		Option<bool> operator()(const T &lhs, const T &rhs) const {
			return lhs < rhs;
		}
	};

	template <typename T>
	struct FallibleGt {
		Option<bool> operator()(const T &lhs, const T &rhs) const {
			return lhs > rhs;
		}
	};

	template <typename T>
	struct FallibleEq {
		Option<bool> operator()(const T &lhs, const T &rhs) const {
			return lhs == rhs;
		}
	};

	template <typename T>
	struct FallibleCmp {
		Option<int> operator()(const T &lhs, const T &rhs) const {
#if __cplusplus >= 202002L
			return lhs <=> rhs;
#else
			if (lhs < rhs)
				return -1;
			if (lhs > rhs)
				return 1;
			return 0;
#endif
		}
	};
}

#endif
