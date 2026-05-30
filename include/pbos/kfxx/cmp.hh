#ifndef _PBOS_KFXX_CMP_H_
#define _PBOS_KFXX_CMP_H_

#if __cplusplus >= 202002L
	#include <compare>
#endif
#include "basedefs.hh"
#include "option.hh"

namespace kfxx {
	template <typename T>
	struct cmp {
		int operator()(const T &lhs, const T &rhs) const {
			if (lhs < rhs)
				return -1;
			if (rhs < lhs)
				return 1;
			return 0;
		}
	};
}

#endif
