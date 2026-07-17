#ifndef _FREESTDC_CXX_RANGES_ACCESS_
#define _FREESTDC_CXX_RANGES_ACCESS_

#include <type_traits>

namespace std {
	namespace ranges {
		struct _begin_t {
			template <typename _Tp>
				requires requires(_Tp &&_t) { _t.begin(); }
			constexpr auto operator()(_Tp &&_t) const
				noexcept(noexcept(_t.begin()))
					-> decltype(_t.begin()) {
				return _t.begin();
			}

			template <typename _Tp>
				requires requires(_Tp &&_t) { begin(_t); }
			constexpr auto operator()(_Tp &&_t) const
				noexcept(noexcept(begin(_t)))
					-> decltype(begin(_t)) {
				return begin(_t);
			}
		};
		inline constexpr _begin_t begin{};

		struct _end_t {
			template <typename _Tp>
				requires requires(_Tp &&_t) { _t.end(); }
			constexpr auto operator()(_Tp &&_t) const
				noexcept(noexcept(_t.end()))
					-> decltype(_t.end()) {
				return _t.end();
			}

			template <typename _Tp>
				requires requires(_Tp &&_t) { end(_t); }
			constexpr auto operator()(_Tp &&_t) const
				noexcept(noexcept(end(_t)))
					-> decltype(end(_t)) {
				return end(_t);
			}
		};
		inline constexpr _end_t end{};

		struct _cbegin_t {
			template <typename _Tp>
				requires requires(const _Tp &_t) { _t.begin(); }
			constexpr auto operator()(const _Tp &_t) const
				noexcept(noexcept(_t.begin()))
					-> decltype(_t.begin()) {
				return _t.begin();
			}

			template <typename _Tp>
				requires requires(const _Tp &_t) { begin(_t); }
			constexpr auto operator()(const _Tp &_t) const
				noexcept(noexcept(begin(_t)))
					-> decltype(begin(_t)) {
				return begin(_t);
			}
		};
		inline constexpr _cbegin_t cbegin{};

		struct _cend_t {
			template <typename _Tp>
				requires requires(const _Tp &_t) { _t.end(); }
			constexpr auto operator()(const _Tp &_t) const
				noexcept(noexcept(_t.end()))
					-> decltype(_t.end()) {
				return _t.end();
			}

			template <typename _Tp>
				requires requires(const _Tp &_t) { end(_t); }
			constexpr auto operator()(const _Tp &_t) const
				noexcept(noexcept(end(_t)))
					-> decltype(end(_t)) {
				return end(_t);
			}
		};
		inline constexpr _cend_t cend{};

		struct _rbegin_t {
			template <typename _Tp>
				requires requires(_Tp &&_t) { _t.rbegin(); }
			constexpr auto operator()(_Tp &&_t) const
				noexcept(noexcept(_t.rbegin()))
					-> decltype(_t.rbegin()) {
				return _t.rbegin();
			}

			template <typename _Tp>
				requires requires(_Tp &&_t) { rbegin(_t); }
			constexpr auto operator()(_Tp &&_t) const
				noexcept(noexcept(rbegin(_t)))
					-> decltype(rbegin(_t)) {
				return rbegin(_t);
			}
		};
		inline constexpr _rbegin_t rbegin{};

		struct _rend_t {
			template <typename _Tp>
				requires requires(_Tp &&_t) { _t.rend(); }
			constexpr auto operator()(_Tp &&_t) const
				noexcept(noexcept(_t.rend()))
					-> decltype(_t.rend()) {
				return _t.rend();
			}

			template <typename _Tp>
				requires requires(_Tp &&_t) { rend(_t); }
			constexpr auto operator()(_Tp &&_t) const
				noexcept(noexcept(rend(_t)))
					-> decltype(rend(_t)) {
				return rend(_t);
			}
		};
		inline constexpr _rend_t rend{};

		struct _crbegin_t {
			template <typename _Tp>
				requires requires(const _Tp &_t) { _t.rbegin(); }
			constexpr auto operator()(const _Tp &_t) const
				noexcept(noexcept(_t.rbegin()))
					-> decltype(_t.rbegin()) {
				return _t.rbegin();
			}

			template <typename _Tp>
				requires requires(const _Tp &_t) { rbegin(_t); }
			constexpr auto operator()(const _Tp &_t) const
				noexcept(noexcept(rbegin(_t)))
					-> decltype(rbegin(_t)) {
				return rbegin(_t);
			}
		};
		inline constexpr _crbegin_t crbegin{};

		struct _crend_t {
			template <typename _Tp>
				requires requires(const _Tp &_t) { _t.rend(); }
			constexpr auto operator()(const _Tp &_t) const
				noexcept(noexcept(_t.rend()))
					-> decltype(_t.rend()) {
				return _t.rend();
			}

			template <typename _Tp>
				requires requires(const _Tp &_t) { rend(_t); }
			constexpr auto operator()(const _Tp &_t) const
				noexcept(noexcept(rend(_t)))
					-> decltype(rend(_t)) {
				return rend(_t);
			}
		};
		inline constexpr _crend_t crend{};

		struct _size_t {
			template <typename _Tp>
				requires requires(const _Tp &_t) { _t.size(); }
			constexpr auto operator()(const _Tp &_t) const
				noexcept(noexcept(_t.size()))
					-> decltype(_t.size()) {
				return _t.size();
			}

			template <typename _Tp>
				requires requires(const _Tp &_t) { size(_t); }
			constexpr auto operator()(const _Tp &_t) const
				noexcept(noexcept(size(_t)))
					-> decltype(size(_t)) {
				return size(_t);
			}
		};
		inline constexpr _size_t size{};

		struct _ssize_t {
			template <typename _Tp>
				requires requires(const _Tp &_t) { ranges::size(_t); }
			constexpr auto operator()(const _Tp &_t) const
				noexcept(noexcept(ranges::size(_t)))
					-> make_signed_t<decltype(ranges::size(_t))> {
				return static_cast<make_signed_t<decltype(ranges::size(_t))>>(ranges::size(_t));
			}
		};
		inline constexpr _ssize_t ssize{};

		struct _empty_t {
			template <typename _Tp>
				requires requires(const _Tp &_t) { bool(_t.empty()); }
			constexpr bool operator()(const _Tp &_t) const
				noexcept(noexcept(bool(_t.empty()))) {
				return bool(_t.empty());
			}

			template <typename _Tp>
				requires requires(const _Tp &_t) { ranges::size(_t); }
			constexpr bool operator()(const _Tp &_t) const
				noexcept(noexcept(ranges::size(_t) == 0)) {
				return ranges::size(_t) == 0;
			}
		};
		inline constexpr _empty_t empty{};

		struct _data_t {
			template <typename _Tp>
				requires requires(_Tp &&_t) { _t.data(); }
			constexpr auto operator()(_Tp &&_t) const
				noexcept(noexcept(_t.data()))
					-> decltype(_t.data()) {
				return _t.data();
			}

			template <typename _Tp>
				requires requires(_Tp &&_t) { data(_t); }
			constexpr auto operator()(_Tp &&_t) const
				noexcept(noexcept(data(_t)))
					-> decltype(data(_t)) {
				return data(_t);
			}
		};
		inline constexpr _data_t data{};

		struct _cdata_t {
			template <typename _Tp>
				requires requires(const _Tp &_t) { _t.data(); }
			constexpr auto operator()(const _Tp &_t) const
				noexcept(noexcept(_t.data()))
					-> decltype(_t.data()) {
				return _t.data();
			}

			template <typename _Tp>
				requires requires(const _Tp &_t) { data(_t); }
			constexpr auto operator()(const _Tp &_t) const
				noexcept(noexcept(data(_t)))
					-> decltype(data(_t)) {
				return data(_t);
			}
		};
		inline constexpr _cdata_t cdata{};

#if defined(_cpp_lib_ranges_as_rvalue) || _cplusplus > 202002L
		// C++23: ranges::reserve_hint
		struct _reserve_hint_t {
			template <typename _R, typename _Size>
				requires requires(_R &&_r, _Size _s) {
					_r.reserve_hint(_s);
				}
			constexpr auto operator()(_R &&_r, _Size _s) const
				noexcept(noexcept(_r.reserve_hint(_s)))
					-> decltype(_r.reserve_hint(_s)) {
				return _r.reserve_hint(_s);
			}

			template <typename _R, typename _Size>
				requires requires(_R &&_r, _Size _s) {
					reserve_hint(_r, _s);
				}
			constexpr auto operator()(_R &&_r, _Size _s) const
				noexcept(noexcept(reserve_hint(_r, _s)))
					-> decltype(reserve_hint(_r, _s)) {
				return reserve_hint(_r, _s);
			}
		};
		inline constexpr _reserve_hint_t reserve_hint{};
#endif	// C++23
	}
}

#endif
