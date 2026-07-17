#ifndef _FREESTDC_CXX_ITERATOR_ITER_MOVE_
#define _FREESTDC_CXX_ITERATOR_ITER_MOVE_

#include "concepts.hh"

namespace std {
	namespace ranges {
		namespace _iter_swap_impl {
			void iter_swap();

			struct _IterSwap {
				template <typename _I1, typename _I2>
				constexpr void operator()(_I1 &&_a, _I2 &&_b) const
					noexcept(noexcept(_IterSwap::_impl(static_cast<_I1 &&>(_a), static_cast<_I2 &&>(_b)))) {
					_impl(static_cast<_I1 &&>(_a), static_cast<_I2 &&>(_b));
				}

			private:
				template <typename _I1, typename _I2>
				static constexpr void _impl(_I1 &&_a, _I2 &&_b) noexcept(noexcept(iter_swap(static_cast<_I1 &&>(_a), static_cast<_I2 &&>(_b)))) {
					if constexpr (requires { iter_swap(static_cast<_I1 &&>(_a), static_cast<_I2 &&>(_b)); }) {
						iter_swap(static_cast<_I1 &&>(_a), static_cast<_I2 &&>(_b));
					}

					else if constexpr (indirectly_readable<remove_cvref_t<_I1>> &&
									   indirectly_readable<remove_cvref_t<_I2>> &&
									   swappable_with<iter_reference_t<remove_cvref_t<_I1>>,
										   iter_reference_t<remove_cvref_t<_I2>>>) {
						ranges::swap(*static_cast<_I1 &&>(_a), *static_cast<_I2 &&>(_b));
					} else {
						static_assert(sizeof(_I1) == 0,
							"iter_swap: no ADL iter_swap found and references are not swappable");
					}
				}
			};
		}

		inline namespace _cpo {
			inline constexpr _iter_swap_impl::_IterSwap iter_swap{};
		}
	}
}

#endif
