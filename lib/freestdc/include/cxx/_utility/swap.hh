#ifndef _FREESTDC_CXX_UTILITY_SWAP_H_
#define _FREESTDC_CXX_UTILITY_SWAP_H_

#include "../_concepts/assignable_from.hh"
#include "../_concepts/ctordtor.hh"
#include "../_concepts/same_as.hh"
#include "forward.hh"

namespace std {
	namespace ranges {
		namespace _swap_detail {

			template <typename _Tp, typename _Up>
			concept _has_adl_swap = requires(_Tp &&_t, _Up &&_u) {
				swap(static_cast<_Tp &&>(_t), static_cast<_Up &&>(_u));
			};

			struct _fn {
				template <typename _Tp, typename _Up>
				constexpr void operator()(_Tp &&_t, _Up &&_u) const
					noexcept(noexcept(swap(std::forward<_Tp>(_t), std::forward<_Up>(_u)))) {
					swap(std::forward<_Tp>(_t), std::forward<_Up>(_u));
				}
			};
		}

		struct _swap_t {
			template <typename _Tp, typename _Up>
				requires _swap_detail::_has_adl_swap<_Tp, _Up>
			constexpr void operator()(_Tp &&_t, _Up &&_u) const
				noexcept(noexcept(_swap_detail::_fn{}(std::forward<_Tp>(_t), std::forward<_Up>(_u)))) {
				_swap_detail::_fn{}(std::forward<_Tp>(_t), std::forward<_Up>(_u));
			}

			template <typename _Tp, typename _Up>
				requires(!_swap_detail::_has_adl_swap<_Tp, _Up>) && same_as<remove_reference_t<_Tp>, remove_reference_t<_Up>> && move_constructible<remove_reference_t<_Tp>> && assignable_from<remove_reference_t<_Tp> &, remove_reference_t<_Tp>>
			constexpr void operator()(_Tp &&_t, _Up &&_u) const
				noexcept(is_nothrow_move_constructible_v<remove_reference_t<_Tp>> &&
						 is_nothrow_assignable_v<remove_reference_t<_Tp> &, remove_reference_t<_Tp>>) {
				using _V = remove_reference_t<_Tp>;
				_V _tmp = std::move(_t);

				_t = static_cast<_Tp &&>(_u);

				_u = static_cast<_Up &&>(_tmp);
			}
		};

		inline constexpr _swap_t swap{};
	}
}

#endif
