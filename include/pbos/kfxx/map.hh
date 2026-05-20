#ifndef _PBOS_KFXX_MAP_H_
#define _PBOS_KFXX_MAP_H_

#include "set.hh"
#include "uninit.hh"

namespace kfxx {
	template <typename K, typename V, typename Lt, bool Fallible, bool IsThreeway>
	class map_impl final {
	private:
		static_assert(std::is_move_constructible_v<K>, "The key must be move-constructible");
		static_assert(std::is_move_constructible_v<V>, "The value must be move-constructible");
		struct pair_t {
			uninit_t<K> key;
			uninit_t<V> value;
			bool key_constructed;
			bool value_constructed;
			bool for_query;

			PBOS_FORCEINLINE pair_t() : key_constructed(false), value_constructed(false), for_query(true) {}
			PBOS_FORCEINLINE pair_t(K &&key, V &&value, bool for_query) : key(std::move(key)), value(std::move(value)), for_query(for_query), key_constructed(true), value_constructed(true) {}
			PBOS_FORCEINLINE pair_t(pair_t &&rhs) noexcept : for_query(false) {
				if (rhs.key_constructed) {
					key = std::move(rhs.key.get());
					rhs.key_constructed = false;
					key_constructed = true;
				}
				if (rhs.value_constructed) {
					value = std::move(rhs.value.get());
					rhs.value_constructed = false;
					value_constructed = true;
				}
			}

			PBOS_FORCEINLINE ~pair_t() {
				if (key_constructed)
					key.destroy();
				if (value_constructed)
					value.destroy();
			}
		};

		/// @brief Extended pair used for query by key only.
		struct query_pair_t : public pair_t {
			const K *query_key;

			PBOS_FORCEINLINE query_pair_t(const K *query_key) : pair_t(), query_key(query_key) {}
		};

		/// @brief Wrapped comparator for comparing pairs by key.
		struct pair_cmp_t {
			Lt inner_cmp;

			PBOS_FORCEINLINE pair_cmp_t(Lt &&inner_cmp) : inner_cmp(std::move(inner_cmp)) {}

			PBOS_FORCEINLINE decltype(std::declval<Lt>()(std::declval<K>(), std::declval<K>())) operator()(const pair_t &lhs, const pair_t &rhs) const {
				const K &l = lhs.for_query ? *((const query_pair_t &)lhs).query_key : lhs.key.get(),
						&r = rhs.for_query ? *((const query_pair_t &)rhs).query_key : rhs.key.get();
				return inner_cmp(l, r);
			}

			template <typename U>
			PBOS_FORCEINLINE decltype(std::declval<Lt>()(std::declval<K>(), std::declval<U>())) operator()(const pair_t &lhs, const U &rhs) const {
				const K &l = lhs.for_query ? *((const query_pair_t &)lhs).query_key : lhs.key.get();
				return inner_cmp(l, rhs);
			}

			template <typename U>
			PBOS_FORCEINLINE decltype(std::declval<Lt>()(std::declval<U>(), std::declval<K>())) operator()(const U &lhs, const pair_t &rhs) const {
				const K &r = rhs.for_query ? *((const query_pair_t &)rhs).query_key : rhs.key.get();
				return inner_cmp(lhs, r);
			}
		};

		using Set = std::conditional_t<Fallible, fallible_set<pair_t, pair_cmp_t, IsThreeway>, kfxx::set_t<pair_t, pair_cmp_t, IsThreeway>>;

		Set _set;

		using ThisType = map_impl<K, V, Lt, Fallible, IsThreeway>;

	public:
		using Node = typename Set::Node;

		using RemoveResultType = typename Set::RemoveResultType;
		using ElementQueryResult = typename std::conditional_t<Fallible, option_t<V &>, V &>;
		using ConstElementQueryResult = typename std::conditional_t<Fallible, option_t<const V &>, const V &>;
		using ContainsResultType = typename Set::ContainsResultType;

		PBOS_FORCEINLINE map_impl(allocator_t *allocator, Lt &&comparator = {}) : _set(allocator, pair_cmp_t(std::move(comparator))) {}
		PBOS_FORCEINLINE map_impl(ThisType &&rhs) : _set(std::move(rhs._set)) {
		}
		PBOS_FORCEINLINE ThisType &operator=(ThisType &&rhs) noexcept {
			clear();
			_set = std::move(rhs._set);
			return *this;
		}

		PBOS_FORCEINLINE bool insert(K &&key, V &&value) {
			pair_t pair = pair_t(std::move(key), std::move(value), false);
			return _set.insert(std::move(pair));
		}

		PBOS_FORCEINLINE RemoveResultType remove(const K &key) {
			return _set.remove(query_pair_t(&key));
		}

		template <typename U>
		PBOS_FORCEINLINE RemoveResultType remove_alt(const U &key) {
			return _set.template remove_alt<U>(key);
		}

		PBOS_FORCEINLINE ContainsResultType contains(const K &key) const {
			return _set.contains(query_pair_t(&key));
		}

		template <typename U>
		PBOS_FORCEINLINE ContainsResultType contains_alt(const U &key) const {
			return _set.template contains_alt<U>(key);
		}

		PBOS_FORCEINLINE ElementQueryResult at(const K &key) {
			if constexpr (Fallible) {
				auto v = _set.at(query_pair_t(&key));

				if (!v.has_value())
					return nullopt;

				return v.value().value.get();
			} else {
				return _set.at(query_pair_t(&key)).value.get();
			}
		}

		template <typename U>
		PBOS_FORCEINLINE ElementQueryResult at_alt(const U &key) {
			if constexpr (Fallible) {
				auto v = _set.template at_alt<U>(key);

				if (!v.has_value())
					return nullopt;

				return v.value().value.get();
			} else {
				return _set.template at_alt<U>(key).value.get();
			}
		}

		PBOS_FORCEINLINE ConstElementQueryResult at(const K &key) const {
			if constexpr (Fallible) {
				auto v = _set.at(query_pair_t(&key));

				if (!v.has_value())
					return nullopt;

				return v.value().value.get();
			} else {
				return _set.at(query_pair_t(&key)).value.get();
			}
		}

		template <typename U>
		PBOS_FORCEINLINE ElementQueryResult at_alt(const U &key) const {
			if constexpr (Fallible) {
				auto v = _set.template at_alt<U>(key);

				if (!v.has_value())
					return nullopt;

				return v.value().value.get();
			} else {
				return _set.template at_alt<U>(key).value.get();
			}
		}

		PBOS_FORCEINLINE allocator_t *allocator() const {
			return _set.allocator();
		}

		PBOS_FORCEINLINE void replace_allocator(allocator_t *rhs) noexcept {
			_set.replace_allocator(rhs);
		}

		PBOS_FORCEINLINE Lt &comparator() {
			return _set.comparator().inner_cmp;
		}

		PBOS_FORCEINLINE const Lt &comparator() const {
			return _set.comparator().inner_cmp;
		}

		PBOS_FORCEINLINE void clear() {
			_set.clear();
		}

		PBOS_FORCEINLINE size_t size() {
			return _set.size();
		}

		struct iterator {
			typename Set::iterator _iterator;
			PBOS_FORCEINLINE iterator(typename Set::iterator &&iterator_in) : _iterator(iterator_in) {
			}
			iterator(const iterator &rhs) = default;
			iterator(iterator &&rhs) = default;
			iterator &operator=(const iterator &rhs) = default;
			iterator &operator=(iterator &&rhs) = default;

			PBOS_FORCEINLINE bool operator==(const iterator &rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator==(iterator &&rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(const iterator &rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(iterator &&rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE iterator &operator--() {
				--_iterator;
				return *this;
			}

			PBOS_FORCEINLINE iterator operator--(int) {
				iterator it = *this;
				--*this;
				return it;
			}

			PBOS_FORCEINLINE iterator prev() {
				iterator iterator = *this;

				return --iterator;
			}

			PBOS_FORCEINLINE iterator &operator++() {
				++_iterator;
				return *this;
			}

			PBOS_FORCEINLINE iterator operator++(int) {
				iterator it = *this;
				++*this;
				return it;
			}

			PBOS_FORCEINLINE iterator next() {
				iterator iterator = *this;

				return ++iterator;
			}

			PBOS_FORCEINLINE K &key() const {
				return _iterator->key.get();
			}

			PBOS_FORCEINLINE V &value() const {
				return _iterator->value.get();
			}

			PBOS_FORCEINLINE std::pair<K &, V &> operator*() const {
				return { _iterator->key.get(), _iterator->value.get() };
			}
		};

		iterator begin() {
			return iterator(_set.begin());
		}
		iterator end() {
			return iterator(_set.end());
		}
		iterator begin_reversed() {
			return iterator(_set.begin_reversed());
		}
		iterator end_reversed() {
			return iterator(_set.end_reversed());
		}

		struct const_iterator {
			iterator _iterator;
			PBOS_FORCEINLINE const_iterator(iterator &&iterator_in) : _iterator(iterator_in) {
			}
			const_iterator(const const_iterator &rhs) = default;
			const_iterator(const_iterator &&rhs) = default;
			const_iterator &operator=(const const_iterator &rhs) = default;
			const_iterator &operator=(const_iterator &&rhs) = default;

			PBOS_FORCEINLINE bool operator==(const const_iterator &rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator==(const_iterator &&rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(const const_iterator &rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(const_iterator &&rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE const_iterator &operator++() {
				++_iterator;
				return *this;
			}

			PBOS_FORCEINLINE const_iterator next() {
				const_iterator iterator = *this;

				return ++iterator;
			}

			PBOS_FORCEINLINE const K &key() const {
				return _iterator.key();
			}

			PBOS_FORCEINLINE const V &value() const {
				return _iterator.value();
			}

			PBOS_FORCEINLINE std::pair<const K &, const V &> operator*() const {
				return { _iterator.key(), _iterator.value() };
			}
		};

		PBOS_FORCEINLINE const_iterator begin() const noexcept {
			return const_iterator(const_cast<ThisType *>(this)->begin());
		}
		PBOS_FORCEINLINE const_iterator end() const noexcept {
			return const_iterator(const_cast<ThisType *>(this)->end());
		}
		PBOS_FORCEINLINE const_iterator begin_const() const noexcept {
			return const_iterator(const_cast<ThisType *>(this)->begin());
		}
		PBOS_FORCEINLINE const_iterator end_const() const noexcept {
			return const_iterator(const_cast<ThisType *>(this)->end());
		}
		PBOS_FORCEINLINE const_iterator begin_const_reversed() const noexcept {
			return const_iterator(const_cast<ThisType *>(this)->begin_reversed());
		}
		PBOS_FORCEINLINE const_iterator end_const_reversed() const noexcept {
			return const_iterator(const_cast<ThisType *>(this)->end_reversed());
		}

		PBOS_FORCEINLINE const_iterator find(const K &key) const {
			return const_cast<ThisType *>(this)->find(key);
		}

		template <typename U>
		PBOS_FORCEINLINE const_iterator find_alt(const U &key) const {
			return const_cast<ThisType *>(this)->template find_alt<U>(key);
		}

		PBOS_FORCEINLINE iterator find(const K &key) {
			return iterator(_set.find(query_pair_t(&key)));
		}

		template <typename U>
		PBOS_FORCEINLINE iterator find_alt(const U &key) {
			return iterator(_set.template find_alt<U>(key));
		}

		PBOS_FORCEINLINE const_iterator find_max_lteq(const K &key) const {
			return const_cast<ThisType *>(this)->find_max_lteq(key);
		}

		template <typename U>
		PBOS_FORCEINLINE const_iterator find_max_lteq_alt(const U &key) const {
			return const_cast<ThisType *>(this)->find_max_lteq_alt<U>(key);
		}

		PBOS_FORCEINLINE iterator find_max_lteq(const K &key) {
			return iterator(_set.find_max_lteq(query_pair_t(&key)));
		}

		template <typename U>
		PBOS_FORCEINLINE iterator find_max_lteq_alt(const U &key) {
			return iterator(_set.template find_max_lteq_alt<U>(key));
		}

		PBOS_FORCEINLINE kfxx::option_t<std::pair<K, V>> remove(const iterator &iterator) {
			kfxx::option_t<pair_t> pair = _set.remove(iterator._iterator);
			if (pair.has_value()) {
				assert(((*pair).key_constructed && (*pair).value_constructed) ||
					   ((!(*pair).key_constructed) && (!(*pair).key_constructed)));
				if((*pair).key_constructed) {
					(*pair).key_constructed = false;
					(*pair).value_constructed = false;
					return std::pair<K, V>{ std::move((*pair).key.get()), std::move((*pair).value.get()) };
				}
				return {};
			}
			return {};
		}
	};

	template <typename K, typename V, typename Lt = kfxx::cmp<K>, bool IsThreeway = true>
	using map_t = map_impl<K, V, Lt, false, IsThreeway>;
	template <typename K, typename V, typename Lt = kfxx::fallible_cmp<K>, bool IsThreeway = true>
	using fallible_map_t = map_impl<K, V, Lt, true, IsThreeway>;
}

#endif
