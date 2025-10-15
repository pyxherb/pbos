#ifndef _PBOS_KFXX_RBTREE_H_
#define _PBOS_KFXX_RBTREE_H_

#include "fallible_cmp.hh"
#include <functional>
#include <pbos/km/assert.h>
#include <pbos/km/panic.h>
#include <pbos/km/result.h>

#ifdef __cplusplus
namespace kfxx {
	enum class RBColor {
		Black = 0,
		Red = 1
	};

	class RBTreeBase {
	public:
		struct NodeBase {
			NodeBase *p = nullptr, *l = nullptr, *r = nullptr;
			RBColor color = RBColor::Black;
		};

	protected:
		NodeBase *_root = nullptr;
		NodeBase *_cachedMinNode = nullptr, *_cachedMaxNode = nullptr;
		size_t _nNodes = 0;

		PB_KFXX_API static NodeBase *_getMinNode(NodeBase *node);
		PB_KFXX_API static NodeBase *_getMaxNode(NodeBase *node);

		PB_FORCEINLINE static bool _isRed(NodeBase *node) { return node && node->color == RBColor::Red; }
		PB_FORCEINLINE static bool _isBlack(NodeBase *node) { return (!node) || node->color == RBColor::Black; }

		PB_KFXX_API void _lRot(NodeBase *x);
		PB_KFXX_API void _rRot(NodeBase *x);

		PB_KFXX_API void _insertFixUp(NodeBase *node);

		PB_KFXX_API NodeBase *_removeFixUp(NodeBase *node);

		PB_KFXX_API static NodeBase *_getNextNode(const NodeBase *node, const NodeBase *lastNode) noexcept;
		PB_KFXX_API static NodeBase *_getPrevNode(const NodeBase *node, const NodeBase *firstNode) noexcept;

		PB_KFXX_API RBTreeBase();
		PB_KFXX_API ~RBTreeBase();
	};

	template <typename T,
		typename Comparator,
		bool Fallible>
	PB_REQUIRES_CONCEPT(std::invocable<Comparator, const T &, const T &> &&std::strict_weak_order<Comparator, T, T>)
	class RBTreeImpl : protected RBTreeBase {
	public:
		struct Node : public RBTreeBase::NodeBase {
			T value;
		};

		using NodeType = Node;
		using ComparatorType = Comparator;

	protected:
		using NodeQueryResultType = typename std::conditional<Fallible, Option<Node *>, Node *>::type;

		using ThisType = RBTreeImpl<T, Comparator, Fallible>;

		Comparator _comparator;

		PB_FORCEINLINE NodeQueryResultType _get(const T &key) const {
			Node *i = (Node *)_root;

			if constexpr (Fallible) {
				Option<bool> result;

				while (i) {
					if ((result = _comparator(i->value, key)).hasValue()) {
						if (result.value()) {
	#ifndef _NDEBUG
							if ((result = _comparator(key, i->value)).hasValue()) {
								kd_assert(!result.value());
								i = (Node *)i->r;
							} else {
								return NULL_OPTION;
							}
	#else
							i = (Node *)i->r;
	#endif
						} else if ((result = _comparator(key, i->value)).hasValue()) {
							if (result.value()) {
								i = (Node *)i->l;
							} else
								return i;
						} else {
							return NULL_OPTION;
						}
					} else {
						return NULL_OPTION;
					}
				}
			} else {
				while (i) {
					if (_comparator(i->value, key)) {
						kd_assert(!_comparator(key, i->value));
						i = (Node *)i->r;
					} else if (_comparator(key, i->value)) {
						i = (Node *)i->l;
					} else
						return i;
				}
			}
			return nullptr;
		}

		PB_FORCEINLINE Node **_getSlot(const T &key, Node *&parentOut) {
			Node **i = (Node **)&_root;
			parentOut = nullptr;

			if constexpr (Fallible) {
				Option<bool> result;

				while (*i) {
					parentOut = *i;

					if ((result = _comparator((*i)->value, key)).hasValue()) {
						if (result.value()) {
	#ifndef _NDEBUG
							if ((result = _comparator(key, (*i)->value)).hasValue()) {
								kd_assert(!result.value());
								i = (Node **)&(*i)->r;
							} else {
								return nullptr;
							}
	#else
							i = (Node **)&(*i)->r;
	#endif
						} else if ((result = _comparator(key, (*i)->value)).hasValue()) {
							if (result.value()) {
								i = (Node **)&(*i)->l;
							} else
								return i;
						} else {
							return nullptr;
						}
					} else {
						return nullptr;
					}
				}
			} else {
				while (*i) {
					parentOut = *i;

					if (_comparator((*i)->value, key)) {
						kd_assert(!_comparator(key, (*i)->value));
						i = (Node **)&((*i)->r);
					} else if (_comparator(key, (*i)->value)) {
						i = (Node **)&((*i)->l);
					} else
						return nullptr;
				}
			}
			return i;
		}

		PB_FORCEINLINE bool _insert(Node **slot, Node *parent, Node *node) {
			kd_assert(!node->l);
			kd_assert(!node->r);

			if (!_root) {
				_root = node;
				node->color = RBColor::Black;
				goto updateNodeCaches;
			}

			if constexpr (Fallible) {
				{
					Option<bool> result;
					if ((result = _comparator(node->value, parent->value)).hasValue()) {
						if (result.value())
							parent->l = node;
						else
							parent->r = node;
					} else {
						return false;
					}
					node->p = parent;
					node->color = RBColor::Red;

					_insertFixUp(node);
				}
			} else {
				{
					if (_comparator(node->value, parent->value))
						parent->l = node;
					else
						parent->r = node;
					node->p = parent;
					node->color = RBColor::Red;

					_insertFixUp(node);
				}
			}

		updateNodeCaches:
			_cachedMinNode = _getMinNode(_root);
			_cachedMaxNode = _getMaxNode(_root);

			++_nNodes;

			return true;
		}

		PB_FORCEINLINE Node *_remove(Node *node) {
			Node *y = (Node *)_removeFixUp(node);

			_cachedMinNode = _getMinNode(_root);
			_cachedMaxNode = _getMaxNode(_root);

			--_nNodes;

			return y;
		}

	public:
		PB_FORCEINLINE RBTreeImpl(Comparator &&comparator = {}) : _comparator(std::move(comparator)) {}

		PB_FORCEINLINE RBTreeImpl(ThisType &&other)
			: _comparator(std::move(other._comparator)) {
			_root = other._root;
			_cachedMinNode = other._cachedMinNode;
			_cachedMaxNode = other._cachedMaxNode;
			_nNodes = other._nNodes;

			other._root = nullptr;
			other._cachedMinNode = nullptr;
			other._cachedMaxNode = nullptr;
			other._nNodes = 0;
		}

		inline ~RBTreeImpl() {
			if (_root)
				km_panic("Destructing a non-empty RBTree is not allowed");
		}

		PB_FORCEINLINE ThisType &operator=(ThisType &&other) noexcept {
			clear();

			_root = other._root;
			_cachedMinNode = other._cachedMinNode;
			_cachedMaxNode = other._cachedMaxNode;
			_nNodes = other._nNodes;
			_comparator = std::move(other._comparator);

			other._root = nullptr;
			other._cachedMinNode = nullptr;
			other._cachedMaxNode = nullptr;
			other._nNodes = 0;

			return *this;
		}

		PB_FORCEINLINE Node *getMaxLteqNode(const T &data) {
			Node *curNode = (Node *)_root, *maxNode = NULL;

			if constexpr (Fallible) {
				Option<bool> result;

				while (curNode) {
					if ((result = _comparator(curNode->value, data)).hasValue()) {
						if (result.value()) {
							if ((result = _comparator(data, curNode->value)).hasValue()) {
								kd_assert(!result.value());
								maxNode = curNode;
								curNode = (Node *)curNode->r;
							} else {
								return nullptr;
							}
						} else if ((result = _comparator(data, curNode->value)).hasValue()) {
							if (result.value()) {
								curNode = (Node *)curNode->l;
							} else {
								return curNode;
							}
						} else {
							return nullptr;
						}
					} else {
						return nullptr;
					}
				}
			} else {
				while (curNode) {
					if (_comparator(curNode->value, data)) {
						kd_assert(!_comparator(data, curNode->value));
						maxNode = curNode;
						curNode = (Node *)curNode->r;
					} else if (_comparator(data, curNode->value)) {
						curNode = (Node *)curNode->l;
					} else
						return curNode;
				}
			}

			return maxNode;
		}

		PB_FORCEINLINE NodeQueryResultType get(const T &key) const {
			return _get(key);
		}

		/// @brief Insert a node into the tree.
		/// @param node Node to be inserted.
		/// @return Whether the node is inserted successfully, false if node with the same key presents.
		PB_FORCEINLINE void insert(Node *node) {
			Node *parent = nullptr, **slot = _getSlot(node->value, parent);

			if (!slot)
				km_panic("Duplicated node insertion in RBTree is not allowed");

			_insert(slot, parent, node);
		}

		PB_FORCEINLINE void remove(Node *node) {
			Node *y = _remove(node);

			y->l = nullptr;
			y->r = nullptr;
			y->p = nullptr;
		}

		PB_FORCEINLINE void clear() {
			if (_root) {
				_deleteNodeTree((Node *)_root);
				_root = nullptr;
				_nNodes = 0;
			}
			_cachedMaxNode = nullptr;
			_cachedMinNode = nullptr;
		}

		PB_FORCEINLINE size_t size() {
			return _nNodes;
		}

		PB_FORCEINLINE Comparator &comparator() {
			return _comparator;
		}

		PB_FORCEINLINE const Comparator &comparator() const {
			return _comparator;
		}

		static Node *getNextNode(const Node *node, const Node *lastNode) noexcept {
			return (Node *)_getNextNode((const NodeBase *)node, (const NodeBase *)lastNode);
		}

		static Node *getPrevNode(const Node *node, const Node *firstNode) noexcept {
			return (Node *)_getPrevNode((const NodeBase *)node, (const NodeBase *)firstNode);
		}

		struct Iterator {
			Node *node;
			ThisType *tree;
			IteratorDirection direction;

			PB_FORCEINLINE Iterator(
				Node *node,
				ThisType *tree,
				IteratorDirection direction)
				: node(node),
				  tree(tree),
				  direction(direction) {}

			Iterator(const Iterator &it) = default;
			PB_FORCEINLINE Iterator(Iterator &&it) {
				node = it.node;
				tree = it.tree;
				direction = it.direction;

				it.direction = IteratorDirection::Invalid;
			}
			PB_FORCEINLINE Iterator &operator=(const Iterator &rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible iterator direction");
				node = rhs.node;
				tree = rhs.tree;
				return *this;
			}
			PB_FORCEINLINE Iterator &operator=(Iterator &&rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible iterator direction");
				constructAt<Iterator>(this, std::move(rhs));
				return *this;
			}

			PB_FORCEINLINE bool copy(Iterator &dest) noexcept {
				dest = *this;
				return true;
			}

			PB_FORCEINLINE Iterator &operator++() {
				if (!node)
					km_panic("Increasing the end iterator");

				if (direction == IteratorDirection::Forward) {
					node = ThisType::getNextNode(node, nullptr);
				} else {
					node = ThisType::getPrevNode(node, nullptr);
				}

				return *this;
			}

			PB_FORCEINLINE Iterator operator++(int) {
				Iterator it = *this;
				++(*this);
				return it;
			}

			PB_FORCEINLINE Iterator &operator--() {
				if (direction == IteratorDirection::Forward) {
					if (node == tree->_cachedMinNode)
						km_panic("Dereasing the begin iterator");

					node = ThisType::getNextNode(node, nullptr);
				} else {
					if (node == tree->_cachedMaxNode)
						km_panic("Dereasing the begin iterator");

					node = ThisType::getPrevNode(node, nullptr);
				}

				return *this;
			}

			PB_FORCEINLINE Iterator operator--(int) {
				Iterator it = *this;
				--(*this);
				return it;
			}

			PB_FORCEINLINE bool operator==(const Node *node) const noexcept {
				return node == node;
			}

			PB_FORCEINLINE bool operator==(const Iterator &it) const {
				if (tree != it.tree)
					km_panic("Cannot compare iterators from different trees");
				return node == it.node;
			}

			PB_FORCEINLINE bool operator==(const Iterator &&rhs) const {
				const Iterator it = rhs;
				return *this == it;
			}

			PB_FORCEINLINE bool operator!=(const Node *node) const noexcept {
				return node != node;
			}

			PB_FORCEINLINE bool operator!=(const Iterator &it) const {
				if (tree != it.tree)
					km_panic("Cannot compare iterators from different trees");
				return node != it.node;
			}

			PB_FORCEINLINE bool operator!=(Iterator &&rhs) const {
				Iterator it = rhs;
				return *this != it;
			}

			PB_FORCEINLINE T &operator*() {
				if (!node)
					km_panic("Deferencing the end iterator");
				return node->value;
			}

			PB_FORCEINLINE T &operator*() const {
				if (!node)
					km_panic("Deferencing the end iterator");
				return node->value;
			}

			PB_FORCEINLINE T *operator->() {
				if (!node)
					km_panic("Deferencing the end iterator");
				return &node->value;
			}

			PB_FORCEINLINE T *operator->() const {
				if (!node)
					km_panic("Deferencing the end iterator");
				return &node->value;
			}
		};

		PB_FORCEINLINE Iterator begin() {
			return Iterator((Node *)_getMinNode(_root), this, IteratorDirection::Forward);
		}
		PB_FORCEINLINE Iterator end() {
			return Iterator(nullptr, this, IteratorDirection::Forward);
		}
		PB_FORCEINLINE Iterator beginReversed() {
			return Iterator((Node *)_cachedMaxNode, this, IteratorDirection::Reversed);
		}
		PB_FORCEINLINE Iterator endReversed() {
			return Iterator(nullptr, this, IteratorDirection::Reversed);
		}

		struct ConstIterator {
			Iterator _iterator;

			PB_FORCEINLINE ConstIterator(
				Iterator &&iterator)
				: _iterator(iterator) {}

			ConstIterator(const ConstIterator &it) = default;
			PB_FORCEINLINE ConstIterator(ConstIterator &&it) : _iterator(std::move(it._iterator)) {
			}
			PB_FORCEINLINE ConstIterator &operator=(const ConstIterator &rhs) noexcept {
				_iterator = rhs._iterator;
				return *this;
			}
			PB_FORCEINLINE ConstIterator &operator=(ConstIterator &&rhs) noexcept {
				_iterator = std::move(rhs._iterator);
				return *this;
			}

			PB_FORCEINLINE bool copy(ConstIterator &dest) noexcept {
				constructAt<ConstIterator>(&dest, *this);
				return true;
			}

			PB_FORCEINLINE ConstIterator &operator++() {
				++_iterator;
				return *this;
			}

			PB_FORCEINLINE ConstIterator operator++(int) {
				ConstIterator it = *this;
				++(*this);
				return it;
			}

			PB_FORCEINLINE ConstIterator &operator--() {
				--_iterator;
				return *this;
			}

			PB_FORCEINLINE ConstIterator operator--(int) {
				ConstIterator it = *this;
				--(*this);
				return it;
			}

			PB_FORCEINLINE bool operator==(const ConstIterator &it) const {
				return _iterator == it._iterator;
			}

			PB_FORCEINLINE bool operator!=(const ConstIterator &it) const {
				return _iterator != it._iterator;
			}

			PB_FORCEINLINE bool operator==(const Node *node) const {
				return _iterator == node;
			}

			PB_FORCEINLINE bool operator!=(ConstIterator &&rhs) const {
				ConstIterator it = rhs;
				return *this != it;
			}

			PB_FORCEINLINE const T &operator*() {
				return *_iterator;
			}

			PB_FORCEINLINE const T &operator*() const {
				return *_iterator;
			}

			PB_FORCEINLINE const T *operator->() {
				return &*_iterator;
			}

			PB_FORCEINLINE const T *operator->() const {
				return &*_iterator;
			}
		};

		PB_FORCEINLINE ConstIterator beginConst() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->begin());
		}
		PB_FORCEINLINE ConstIterator endConst() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->end());
		}
		PB_FORCEINLINE ConstIterator beginConstReversed() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->beginReversed());
		}
		PB_FORCEINLINE ConstIterator endConstReversed() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->endReversed());
		}

		PB_FORCEINLINE void remove(const Iterator &iterator) {
			kd_assert(("Cannot remove the end iterator", iterator.node));
			remove(iterator.node);
		}
	};

	template <typename T, typename Comparator = std::less<T>>
	using RBTree = RBTreeImpl<T, Comparator, false>;
	template <typename T, typename Comparator = FallibleLt<T>>
	using FallibleRBTree = RBTreeImpl<T, Comparator, true>;
}
#endif

#endif
