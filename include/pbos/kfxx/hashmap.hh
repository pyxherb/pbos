#ifndef _PBOS_KFXX_HASHMAP_HH_
#define _PBOS_KFXX_HASHMAP_HH_

#include "hashset.hh"
#include "uninit.hh"
#include <utility>

namespace kfxx {
	template <typename K, typename V, typename Eq, typename Hasher, bool Fallible>
	// PBOS_REQUIRES_CONCEPT(std::invocable<Eq, const K &, const K &>)
	class HashMapImpl final {
	private:
		static_assert(std::is_move_constructible_v<K>, "The key must be move-constructible");
		static_assert(std::is_move_constructible_v<V>, "The value must be move-constructible");
		struct Pair {
			Uninit<K> key;
			Uninit<V> value;
			bool key_constructed;
			bool value_constructed;
			bool is_for_query;

			PBOS_FORCEINLINE Pair() : key_constructed(false), value_constructed(false), is_for_query(true) {}
			PBOS_FORCEINLINE Pair(K &&key, V &&value, bool is_for_query) : key(std::move(key)), value(std::move(value)), is_for_query(is_for_query), key_constructed(true), value_constructed(true) {}
			PBOS_FORCEINLINE Pair(Pair &&rhs) noexcept: is_for_query(false) {
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

			PBOS_FORCEINLINE ~Pair() {
				if (key_constructed)
					key.destroy();
				if (value_constructed)
					value.destroy();
			}
		};

		struct QueryPair : public Pair {
			const K *query_key;

			PBOS_FORCEINLINE QueryPair(const K *query_key): Pair(), query_key(query_key) {}
		};

		struct PairCmp {
			Eq eq_comparator;

			PBOS_FORCEINLINE decltype(std::declval<Eq>()(std::declval<K>(), std::declval<K>())) operator()(const Pair &lhs, const Pair &rhs) const {
				const K &l = lhs.is_for_query ? *((const QueryPair &)lhs).query_key : lhs.key.get(),
						&r = rhs.is_for_query ? *((const QueryPair &)rhs).query_key : rhs.key.get();
				return eq_comparator(l, r);
			}
		};

		struct PairHasher {
			Hasher hasher;

			PBOS_FORCEINLINE decltype(std::declval<Hasher>()(std::declval<K>())) operator()(const Pair &pair) const {
				const K &k = pair.is_for_query ? *((const QueryPair &)pair).query_key : pair.key.get();
				return hasher(k);
			}
		};

		PairCmp comparator;

		using SetType = std::conditional_t<Fallible, FallibleHashSet<Pair, PairCmp, PairHasher>, HashSet<Pair, PairCmp, PairHasher>>;

		SetType _set;

		using ThisType = HashMapImpl<K, V, Eq, Hasher, Fallible>;

	public:
		using RemoveResultType = typename SetType::RemoveResultType;
		using ElementQueryResultType = typename std::conditional_t<Fallible, Option<V &>, V &>;
		using ConstElementQueryResultType = typename std::conditional_t<Fallible, Option<const V &>, const V &>;
		using ContainsResultType = typename SetType::ContainsResultType;

		PBOS_FORCEINLINE HashMapImpl(Alloc *allocator) : _set(allocator) {}
		PBOS_FORCEINLINE HashMapImpl(ThisType &&rhs) : comparator(std::move(rhs.comparator)), _set(std::move(rhs._set)) {
		}

		PBOS_FORCEINLINE ThisType &operator=(ThisType &&rhs) noexcept {
			clear_and_shrink();

			comparator = std::move(rhs.comparator);
			_set = std::move(rhs._set);

			return *this;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert_without_resize_buckets(K &&key, V &&value) {
			Pair pair = Pair(std::move(key), std::move(value), false);
			return _set.insert_without_resize_buckets(std::move(pair));
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert(K &&key, V &&value) {
			Pair pair = Pair(std::move(key), std::move(value), false);
			return _set.insert(std::move(pair));
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert_and_fetch_key_without_resize_buckets(K &&key, V &&value) {
			Pair pair = Pair(std::move(key), std::move(value), false);
			return _set.insert_without_resize_buckets(std::move(pair));
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert_and_fetch_key(K &&key, V &&value) {
			Pair pair = Pair(std::move(key), std::move(value), false);
			return _set.insert(std::move(pair));
		}

		[[nodiscard]] PBOS_FORCEINLINE RemoveResultType remove(const K &key) {
			if constexpr (Fallible) {
				return _set.remove(QueryPair(&key));
			} else {
				_set.remove(QueryPair(&key));
			}
		}

		PBOS_FORCEINLINE ContainsResultType contains(const K &key) const {
			return _set.contains(QueryPair(&key));
		}

		PBOS_FORCEINLINE ElementQueryResultType at(const K &key) {
			if constexpr (Fallible) {
				auto maybe_pair = _set.at(QueryPair(&key));

				if (!maybe_pair.has_value())
					return NULL_OPTION;

				return maybe_pair.value().value.get();
			} else {
				return _set.at(QueryPair(&key)).value.get();
			}
		}

		PBOS_FORCEINLINE ConstElementQueryResultType at(const K &key) const {
			if constexpr (Fallible) {
				auto maybe_pair = _set.at(QueryPair(&key));

				if (!maybe_pair.has_value())
					return NULL_OPTION;

				return maybe_pair.value().value.get();
			} else {
				return _set.at(QueryPair(&key)).value.get();
			}
		}

		PBOS_FORCEINLINE Alloc *allocator() const {
			return _set.allocator();
		}

		PBOS_FORCEINLINE void replace_allocator(Alloc *rhs) noexcept {
			_set.replace_allocator(rhs);
		}

		PBOS_FORCEINLINE void clear() {
			_set.clear();
		}

		PBOS_FORCEINLINE void clear_and_shrink() {
			_set.clear_and_shrink();
		}

		struct Iterator {
			typename SetType::Iterator _iterator;
			PBOS_FORCEINLINE Iterator(typename SetType::Iterator &&iterator_in) : _iterator(iterator_in) {
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

			PBOS_FORCEINLINE Iterator &operator++() {
				++_iterator;
				return *this;
			}

			PBOS_FORCEINLINE Iterator operator++(int) {
				Iterator it = *this;
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

		PBOS_FORCEINLINE ConstIterator begin_const() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->begin());
		}
		PBOS_FORCEINLINE ConstIterator end_const() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->end());
		}
		PBOS_FORCEINLINE ConstIterator begin_const_reversed() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->begin_reversed());
		}
		PBOS_FORCEINLINE ConstIterator end_const_reversed() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->end_reversed());
		}
		PBOS_FORCEINLINE ConstIterator begin() const {
			return begin_const();
		}
		PBOS_FORCEINLINE ConstIterator end() const {
			return end_const();
		}
		PBOS_FORCEINLINE ConstIterator begin_reversed() const {
			return begin_const_reversed();
		}
		PBOS_FORCEINLINE ConstIterator end_reversed() const {
			return end_const_reversed();
		}

		PBOS_FORCEINLINE ConstIterator find(const K &key) const {
			return ConstIterator(const_cast<ThisType *>(this)->find(key));
		}

		PBOS_FORCEINLINE Iterator find(const K &key) {
			return Iterator(_set.find(QueryPair(&key)));
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

	template <typename K, typename V, typename Eq = std::equal_to<K>, typename Hasher = kfxx::Hash<K>>
	using HashMap = HashMapImpl<K, V, Eq, Hasher, false>;
	template <typename K, typename V, typename Eq = FallibleEq<K>, typename Hasher = kfxx::Hash<K>>
	using FallibleHashMap = HashMapImpl<K, V, Eq, Hasher, true>;
}

#endif
