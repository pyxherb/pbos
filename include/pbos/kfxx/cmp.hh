#ifndef _PBOS_KFXX_CMP_H_
#define _PBOS_KFXX_CMP_H_

#include "basedefs.hh"
#include "option.hh"

namespace kfxx {
	template <typename T>
	struct cmp {
		int operator()(const T &lhs, const T &rhs) const {
#if __cplusplus >= 202002L
			return lhs <=> rhs;
#else
			if (lhs < rhs)
				return -1;
			if (rhs < lhs)
				return 1;
			return 0;
#endif
		}
	};
}

#endif
