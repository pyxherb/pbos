#ifndef _PBOS_KFXX_MAP_H_
#define _PBOS_KFXX_MAP_H_

#include "set.hh"
#include "uninit.hh"

namespace kfxx {
	template <typename K, typename V, typename Lt, bool Fallible, bool IsThreeway>
	class _map_impl final {
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

		using set_t = std::conditional_t<Fallible, fallible_set_t<pair_t, pair_cmp_t, IsThreeway>, set_t<pair_t, pair_cmp_t, IsThreeway>>;

		set_t _set;

		using this_t = _map_impl<K, V, Lt, Fallible, IsThreeway>;

	public:
		using node_t = typename set_t::node_t;

		using remove_result_t = typename set_t::remove_result_t;
		using element_query_result_t = typename std::conditional_t<Fallible, option_t<V &>, V &>;
		using const_element_query_result_t = typename std::conditional_t<Fallible, option_t<const V &>, const V &>;
		using contains_result_t = typename set_t::contains_result_t;

		PBOS_FORCEINLINE _map_impl(allocator_t *allocator, Lt &&comparator = {}) : _set(allocator, pair_cmp_t(std::move(comparator))) {}
		PBOS_FORCEINLINE _map_impl(this_t &&rhs) : _set(std::move(rhs._set)) {
		}
		PBOS_FORCEINLINE this_t &operator=(this_t &&rhs) noexcept {
			clear();
			_set = std::move(rhs._set);
			return *this;
		}

		PBOS_FORCEINLINE bool insert(K &&key, V &&value) {
			pair_t pair = pair_t(std::move(key), std::move(value), false);
			return _set.insert(std::move(pair));
		}

		PBOS_FORCEINLINE remove_result_t remove(const K &key) {
			return _set.remove(query_pair_t(&key));
		}

		template <typename U>
		PBOS_FORCEINLINE remove_result_t remove_alt(const U &key) {
			return _set.template remove_alt<U>(key);
		}

		PBOS_FORCEINLINE contains_result_t contains(const K &key) const {
			return _set.contains(query_pair_t(&key));
		}

		template <typename U>
		PBOS_FORCEINLINE contains_result_t contains_alt(const U &key) const {
			return _set.template contains_alt<U>(key);
		}

		PBOS_FORCEINLINE element_query_result_t at(const K &key) {
			if constexpr (Fallible) {
				auto v = _set.at(query_pair_t(&key));

				if (!v.has_value())
					return NULL_OPTION;

				return v.value().value.get();
			} else {
				return _set.at(query_pair_t(&key)).value.get();
			}
		}

		template <typename U>
		PBOS_FORCEINLINE element_query_result_t at_alt(const U &key) {
			if constexpr (Fallible) {
				auto v = _set.template at_alt<U>(key);

				if (!v.has_value())
					return NULL_OPTION;

				return v.value().value.get();
			} else {
				return _set.template at_alt<U>(key).value.get();
			}
		}

		PBOS_FORCEINLINE const_element_query_result_t at(const K &key) const {
			if constexpr (Fallible) {
				auto v = _set.at(query_pair_t(&key));

				if (!v.has_value())
					return NULL_OPTION;

				return v.value().value.get();
			} else {
				return _set.at(query_pair_t(&key)).value.get();
			}
		}

		template <typename U>
		PBOS_FORCEINLINE element_query_result_t at_alt(const U &key) const {
			if constexpr (Fallible) {
				auto v = _set.template at_alt<U>(key);

				if (!v.has_value())
					return NULL_OPTION;

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

		struct Iterator {
			typename set_t::Iterator _iterator;
			PBOS_FORCEINLINE Iterator(typename set_t::Iterator &&iterator_in) : _iterator(iterator_in) {
			}
			Iterator(const Iterator &rhs) = default;
			Iterator(Iterator &&rhs) = default;
			Iterator &operator=(const Iterator &rhs) = default;
			Iterator &operator=(Iterator &&rhs) = default;

			PBOS_FORCEINLINE bool operator==(const Iterator &rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator==(Iterator &&rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(const Iterator &rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(Iterator &&rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE Iterator &operator--() {
				--_iterator;
				return *this;
			}

			PBOS_FORCEINLINE Iterator operator--(int) {
				Iterator it = *this;
				--*this;
				return it;
			}

			PBOS_FORCEINLINE Iterator prev() {
				Iterator iterator = *this;

				return --iterator;
			}

			PBOS_FORCEINLINE Iterator &operator++() {
				++_iterator;
				return *this;
			}

			PBOS_FORCEINLINE Iterator operator++(int) {
				Iterator it = *this;
				++*this;
				return it;
			}

			PBOS_FORCEINLINE Iterator next() {
				Iterator iterator = *this;

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

		Iterator begin() {
			return Iterator(_set.begin());
		}
		Iterator end() {
			return Iterator(_set.end());
		}
		Iterator begin_reversed() {
			return Iterator(_set.begin_reversed());
		}
		Iterator end_reversed() {
			return Iterator(_set.end_reversed());
		}

		struct ConstIterator {
			Iterator _iterator;
			PBOS_FORCEINLINE ConstIterator(Iterator &&iterator_in) : _iterator(iterator_in) {
			}
			ConstIterator(const ConstIterator &rhs) = default;
			ConstIterator(ConstIterator &&rhs) = default;
			ConstIterator &operator=(const ConstIterator &rhs) = default;
			ConstIterator &operator=(ConstIterator &&rhs) = default;

			PBOS_FORCEINLINE bool operator==(const ConstIterator &rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator==(ConstIterator &&rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(const ConstIterator &rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(ConstIterator &&rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE ConstIterator &operator++() {
				++_iterator;
				return *this;
			}

			PBOS_FORCEINLINE ConstIterator next() {
				ConstIterator iterator = *this;

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

		PBOS_FORCEINLINE ConstIterator begin() const noexcept {
			return ConstIterator(const_cast<this_t *>(this)->begin());
		}
		PBOS_FORCEINLINE ConstIterator end() const noexcept {
			return ConstIterator(const_cast<this_t *>(this)->end());
		}
		PBOS_FORCEINLINE ConstIterator begin_const() const noexcept {
			return ConstIterator(const_cast<this_t *>(this)->begin());
		}
		PBOS_FORCEINLINE ConstIterator end_const() const noexcept {
			return ConstIterator(const_cast<this_t *>(this)->end());
		}
		PBOS_FORCEINLINE ConstIterator begin_const_reversed() const noexcept {
			return ConstIterator(const_cast<this_t *>(this)->begin_reversed());
		}
		PBOS_FORCEINLINE ConstIterator end_const_reversed() const noexcept {
			return ConstIterator(const_cast<this_t *>(this)->end_reversed());
		}

		PBOS_FORCEINLINE ConstIterator find(const K &key) const {
			return const_cast<this_t *>(this)->find(key);
		}

		template <typename U>
		PBOS_FORCEINLINE ConstIterator find_alt(const U &key) const {
			return const_cast<this_t *>(this)->template find_alt<U>(key);
		}

		PBOS_FORCEINLINE Iterator find(const K &key) {
			return Iterator(_set.find(query_pair_t(&key)));
		}

		template <typename U>
		PBOS_FORCEINLINE Iterator find_alt(const U &key) {
			return Iterator(_set.template find_alt<U>(key));
		}

		PBOS_FORCEINLINE ConstIterator find_max_lteq(const K &key) const {
			return const_cast<this_t *>(this)->find_max_lteq(key);
		}

		template <typename U>
		PBOS_FORCEINLINE ConstIterator find_max_lteq_alt(const U &key) const {
			return const_cast<this_t *>(this)->find_max_lteq_alt<U>(key);
		}

		PBOS_FORCEINLINE Iterator find_max_lteq(const K &key) {
			return Iterator(_set.find_max_lteq(query_pair_t(&key)));
		}

		template <typename U>
		PBOS_FORCEINLINE Iterator find_max_lteq_alt(const U &key) {
			return Iterator(_set.template find_max_lteq_alt<U>(key));
		}

		PBOS_FORCEINLINE kfxx::option_t<std::pair<K, V>> remove(const Iterator &iterator) {
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

	template <typename K, typename V, typename Lt = std::less<K>, bool IsThreeway = false>
	using map_t = _map_impl<K, V, Lt, false, IsThreeway>;
	template <typename K, typename V, typename Lt = std::less<K>, bool IsThreeway = false>
	using fallible_map_t = _map_impl<K, V, Lt, true, IsThreeway>;
}

#endif
