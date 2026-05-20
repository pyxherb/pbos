#ifndef _PBOS_KFXX_HASHMAP_HH_
#define _PBOS_KFXX_HASHMAP_HH_

#include "hashset.hh"
#include "uninit.hh"
#include <utility>

namespace kfxx {
	template <typename K, typename V, typename Eq, typename Hasher, bool Fallible>
	// PBOS_REQUIRES_CONCEPT(std::invocable<Eq, const K &, const K &>)
	class hashmap_impl final {
	private:
		static_assert(std::is_move_constructible_v<K>, "The key must be move-constructible");
		static_assert(std::is_move_constructible_v<V>, "The value must be move-constructible");
		struct pair_t {
			uninit_t<K> key;
			uninit_t<V> value;
			bool key_constructed;
			bool value_constructed;
			bool is_for_query;

			PBOS_FORCEINLINE pair_t() : key_constructed(false), value_constructed(false), is_for_query(true) {}
			PBOS_FORCEINLINE pair_t(K &&key, V &&value, bool is_for_query) : key(std::move(key)), value(std::move(value)), is_for_query(is_for_query), key_constructed(true), value_constructed(true) {}
			PBOS_FORCEINLINE pair_t(pair_t &&rhs) noexcept: is_for_query(false) {
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

		struct query_pair_t : public pair_t {
			const K *query_key;

			PBOS_FORCEINLINE query_pair_t(const K *query_key): pair_t(), query_key(query_key) {}
		};

		struct pair_cmp {
			Eq eq_comparator;

			PBOS_FORCEINLINE decltype(std::declval<Eq>()(std::declval<K>(), std::declval<K>())) operator()(const pair_t &lhs, const pair_t &rhs) const {
				const K &l = lhs.is_for_query ? *((const query_pair_t &)lhs).query_key : lhs.key.get(),
						&r = rhs.is_for_query ? *((const query_pair_t &)rhs).query_key : rhs.key.get();
				return eq_comparator(l, r);
			}
		};

		struct pair_hash {
			Hasher hasher;

			PBOS_FORCEINLINE decltype(std::declval<Hasher>()(std::declval<K>())) operator()(const pair_t &pair) const {
				const K &k = pair.is_for_query ? *((const query_pair_t &)pair).query_key : pair.key.get();
				return hasher(k);
			}
		};

		pair_cmp comparator;

		using SetType = std::conditional_t<Fallible, fallible_hashset_t<pair_t, pair_cmp, pair_hash>, hashset_t<pair_t, pair_cmp, pair_hash>>;

		SetType _set;

		using ThisType = hashmap_impl<K, V, Eq, Hasher, Fallible>;

	public:
		using RemoveResultType = typename SetType::RemoveResultType;
		using ElementQueryResultType = typename std::conditional_t<Fallible, option_t<V &>, V &>;
		using ConstElementQueryResultType = typename std::conditional_t<Fallible, option_t<const V &>, const V &>;
		using ContainsResultType = typename SetType::ContainsResultType;

		PBOS_FORCEINLINE hashmap_impl(allocator_t *allocator) : _set(allocator) {}
		PBOS_FORCEINLINE hashmap_impl(ThisType &&rhs) : comparator(std::move(rhs.comparator)), _set(std::move(rhs._set)) {
		}

		PBOS_FORCEINLINE ThisType &operator=(ThisType &&rhs) noexcept {
			clear_and_shrink();

			comparator = std::move(rhs.comparator);
			_set = std::move(rhs._set);

			return *this;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert_without_resize_buckets(K &&key, V &&value) {
			pair_t pair = pair_t(std::move(key), std::move(value), false);
			return _set.insert_without_resize_buckets(std::move(pair));
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert(K &&key, V &&value) {
			pair_t pair = pair_t(std::move(key), std::move(value), false);
			return _set.insert(std::move(pair));
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert_and_fetch_key_without_resize_buckets(K &&key, V &&value) {
			pair_t pair = pair_t(std::move(key), std::move(value), false);
			return _set.insert_without_resize_buckets(std::move(pair));
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert_and_fetch_key(K &&key, V &&value) {
			pair_t pair = pair_t(std::move(key), std::move(value), false);
			return _set.insert(std::move(pair));
		}

		[[nodiscard]] PBOS_FORCEINLINE RemoveResultType remove(const K &key) {
			if constexpr (Fallible) {
				return _set.remove(query_pair_t(&key));
			} else {
				_set.remove(query_pair_t(&key));
			}
		}

		PBOS_FORCEINLINE ContainsResultType contains(const K &key) const {
			return _set.contains(query_pair_t(&key));
		}

		PBOS_FORCEINLINE ElementQueryResultType at(const K &key) {
			if constexpr (Fallible) {
				auto maybe_pair = _set.at(query_pair_t(&key));

				if (!maybe_pair.has_value())
					return nullopt;

				return maybe_pair.value().value.get();
			} else {
				return _set.at(query_pair_t(&key)).value.get();
			}
		}

		PBOS_FORCEINLINE ConstElementQueryResultType at(const K &key) const {
			if constexpr (Fallible) {
				auto maybe_pair = _set.at(query_pair_t(&key));

				if (!maybe_pair.has_value())
					return nullopt;

				return maybe_pair.value().value.get();
			} else {
				return _set.at(query_pair_t(&key)).value.get();
			}
		}

		PBOS_FORCEINLINE allocator_t *allocator() const {
			return _set.allocator();
		}

		PBOS_FORCEINLINE void replace_allocator(allocator_t *rhs) noexcept {
			_set.replace_allocator(rhs);
		}

		PBOS_FORCEINLINE void clear() {
			_set.clear();
		}

		PBOS_FORCEINLINE void clear_and_shrink() {
			_set.clear_and_shrink();
		}

		struct iterator {
			typename SetType::iterator _iterator;
			PBOS_FORCEINLINE iterator(typename SetType::iterator &&iterator_in) : _iterator(iterator_in) {
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

			PBOS_FORCEINLINE iterator &operator++() {
				++_iterator;
				return *this;
			}

			PBOS_FORCEINLINE iterator operator++(int) {
				iterator it = *this;
				++*this;
				return it;
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
		PBOS_FORCEINLINE const_iterator begin() const {
			return begin_const();
		}
		PBOS_FORCEINLINE const_iterator end() const {
			return end_const();
		}
		PBOS_FORCEINLINE const_iterator begin_reversed() const {
			return begin_const_reversed();
		}
		PBOS_FORCEINLINE const_iterator end_reversed() const {
			return end_const_reversed();
		}

		PBOS_FORCEINLINE const_iterator find(const K &key) const {
			return const_iterator(const_cast<ThisType *>(this)->find(key));
		}

		PBOS_FORCEINLINE iterator find(const K &key) {
			return iterator(_set.find(query_pair_t(&key)));
		}

		PBOS_FORCEINLINE size_t size() const {
			return _set.size();
		}

		PBOS_FORCEINLINE bool empty() const {
			return !_set.size();
		}

		PBOS_FORCEINLINE bool shrink_buckets() {
			return _set.shrink_buckets();
		}
	};

	template <typename K, typename V, typename Eq = std::equal_to<K>, typename Hasher = kfxx::hash<K>>
	using hashmap_t = hashmap_impl<K, V, Eq, Hasher, false>;
	template <typename K, typename V, typename Eq = fallible_equal_to<K>, typename Hasher = kfxx::hash<K>>
	using fallible_hashmap_t = hashmap_impl<K, V, Eq, Hasher, true>;
}

#endif
