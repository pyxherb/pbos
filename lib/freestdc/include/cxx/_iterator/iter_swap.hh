#ifndef _FREESTDC_CXX_ITERATOR_ITER_SWAP_
#define _FREESTDC_CXX_ITERATOR_ITER_SWAP_

#include "concepts.hh"

namespace std {
	namespace ranges {
		namespace _iter_move_impl {
			void iter_move();

			struct _IterMove {
				template <typename _Tp>
				constexpr decltype(auto) operator()(_Tp &&_e) const
					noexcept(noexcept(_IterMove::_impl(static_cast<_Tp &&>(_e)))) {
					return _impl(static_cast<_Tp &&>(_e));
				}

			private:
				template <typename _Tp>
				static constexpr decltype(auto) _impl(_Tp &&_e) noexcept(noexcept(iter_move(static_cast<_Tp &&>(_e)))) {
					if constexpr (requires { iter_move(static_cast<_Tp &&>(_e)); }) {
						return iter_move(static_cast<_Tp &&>(_e));
					}

					else if constexpr (input_or_output_iterator<remove_cvref_t<_Tp>>) {
						return std::move(*static_cast<_Tp &&>(_e));
					} else {
						static_assert(sizeof(_Tp) == 0,
							"iter_move: no ADL iter_move found and type does not satisfy input_or_output_iterator");
					}
				}
			};
		}

		inline namespace _cpo {
			inline constexpr _iter_move_impl::_IterMove iter_move{};
		}
	}
}

#endif
