/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    /**
     * In linked_hashmap, iteration ordering is differ from map,
     * which is the order in which keys were inserted into the map.
     * You should maintain a doubly-linked list running through all
     * of its entries to keep the correct iteration order.
     *
     * Note that insertion order is not affected if a key is re-inserted
     * into the map.
     */
    
template<
	class Key,
	class T,
	class Hash = std::hash<Key>, 
	class Equal = std::equal_to<Key>
> class linked_hashmap {
public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::linked_hashmap as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;
 
	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = linked_hashmap.begin(); --it;
	 *       or it = linked_hashmap.end(); ++end();
	 */
    class const_iterator;
    // Intrusive doubly-linked list node for iteration order
    struct Link {
        Link *prev;
        Link *next;
        Link(): prev(this), next(this) {}
    };
    // Hash table node holding key/value and link
    struct Node : Link {
        value_type kv;
        Node *hnext;       // for bucket chain
        Node(const value_type &v) : Link(), kv(v), hnext(nullptr) {}
    };
	class iterator {
	private:
		// pointer to owning container and current node
		friend class linked_hashmap;
		friend class const_iterator;
	public:
		Link *ptr = nullptr;
		const linked_hashmap *owner = nullptr;
	public:
		// The following code is written for the C++ type_traits library.
		// Type traits is a C++ feature for describing certain properties of a type.
		// For instance, for an iterator, iterator::value_type is the type that the 
		// iterator points to. 
		// STL algorithms and containers may use these type_traits (e.g. the following 
		// typedef) to work properly. 
		// See these websites for more information:
		// https://en.cppreference.com/w/cpp/header/type_traits
		// About value_type: https://blog.csdn.net/u014299153/article/details/72419713
		// About iterator_category: https://en.cppreference.com/w/cpp/iterator
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::bidirectional_iterator_tag;


		iterator() = default;
		iterator(const iterator &other) = default;
		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
			if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
			if (ptr == owner->root) throw invalid_iterator();
			iterator old = *this;
			ptr = ptr->next;
			return old;
		}
		/**
		 * TODO ++iter
		 */
		iterator & operator++() {
			if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
			if (ptr == owner->root) throw invalid_iterator();
			ptr = ptr->next;
			return *this;
		}
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
			if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
			iterator old = *this;
			// if at end(), move to last element if exists
			if (ptr == owner->root) {
				if (owner->sz == 0) throw invalid_iterator();
				ptr = owner->root->prev;
				return old;
			}
			// if at begin(), cannot --
			if (ptr->prev == owner->root) throw invalid_iterator();
			ptr = ptr->prev;
			return old;
		}
		/**
		 * TODO --iter
		 */
		iterator & operator--() {
			if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
			if (ptr == owner->root) {
				if (owner->sz == 0) throw invalid_iterator();
				ptr = owner->root->prev;
				return *this;
			}
			if (ptr->prev == owner->root) throw invalid_iterator();
			ptr = ptr->prev;
			return *this;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const { return static_cast<Node*>(ptr)->kv; }
		bool operator==(const iterator &rhs) const { return ptr == rhs.ptr && owner == rhs.owner; }
		bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr && owner == rhs.owner; }
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
		bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }

		/**
		 * for the support of it->first. 
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept { return &(static_cast<Node*>(ptr)->kv); }
	};
 
	class const_iterator {
		// it should has similar member method as iterator.
		//  and it should be able to construct from an iterator.
		private:
			friend class linked_hashmap;
			friend class iterator;
	public:
			const linked_hashmap *owner = nullptr;
			const Link *ptr = nullptr;
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = typename linked_hashmap::value_type;
			using pointer = const value_type*;
			using reference = const value_type&;
			using iterator_category = std::bidirectional_iterator_tag;
			const_iterator() = default;
			const_iterator(const const_iterator &other) = default;
			const_iterator(const iterator &other) { owner = other.owner; ptr = other.ptr; }
			reference operator*() const { return static_cast<const Node*>(ptr)->kv; }
			pointer operator->() const noexcept { return &(static_cast<const Node*>(ptr)->kv); }
			const_iterator operator++(int) {
				if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
				if (ptr == owner->root) throw invalid_iterator();
				const_iterator old = *this;
				ptr = ptr->next;
				return old;
			}
			const_iterator & operator++() {
				if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
				if (ptr == owner->root) throw invalid_iterator();
				ptr = ptr->next;
				return *this;
			}
			const_iterator operator--(int) {
				if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
				const_iterator old = *this;
				if (ptr == owner->root) {
					if (owner->sz == 0) throw invalid_iterator();
					ptr = owner->root->prev;
					return old;
				}
				if (ptr->prev == owner->root) throw invalid_iterator();
				ptr = ptr->prev;
				return old;
			}
			const_iterator & operator--() {
				if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
				if (ptr == owner->root) {
					if (owner->sz == 0) throw invalid_iterator();
					ptr = owner->root->prev;
					return *this;
				}
				if (ptr->prev == owner->root) throw invalid_iterator();
				ptr = ptr->prev;
				return *this;
			}
			bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr && owner == rhs.owner; }
			bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
			bool operator==(const iterator &rhs) const { return ptr == rhs.ptr && owner == rhs.owner; }
			bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
			// And other methods in iterator.
			// And other methods in iterator.
			// And other methods in iterator.
	};
 
	/**
	 * TODO two constructors
	 */
private:

	Hash hasher;
	Equal equaler;
	Node **buckets = nullptr;
	size_t bucket_cnt = 0;
	size_t sz = 0;
	Link *root = nullptr; // sentinel node (no kv)

	static constexpr size_t init_bucket() { return 16; }

	void init_empty(size_t bc = init_bucket()) {
		bucket_cnt = bc;
		buckets = new Node*[bucket_cnt];
		for (size_t i = 0; i < bucket_cnt; ++i) buckets[i] = nullptr;
		root = new Link();
		sz = 0;
	}

	void destroy_all() {
		if (!root) return;
		Link *p = root->next;
		while (p != root) {
			Link *nxt = p->next;
			delete static_cast<Node*>(p);
			p = nxt;
		}
		delete root;
		root = nullptr;
		delete[] buckets;
		buckets = nullptr;
		bucket_cnt = 0;
		sz = 0;
	}

	void relink_in_list_tail(Node *node) {
		// insert before root (tail)
		node->next = root;
		node->prev = root->prev;
		root->prev->next = node;
		root->prev = node;
	}

	void buckets_insert(Node *node) {
		size_t idx = hasher(node->kv.first) % bucket_cnt;
		node->hnext = buckets[idx];
		buckets[idx] = node;
	}

	void rehash_if_needed() {
		if (bucket_cnt == 0) return;
		if (sz * 4 < bucket_cnt * 3) return; // load factor <= 0.75
		size_t new_cnt = bucket_cnt << 1;
		Node **nb = new Node*[new_cnt];
		for (size_t i = 0; i < new_cnt; ++i) nb[i] = nullptr;
		for (size_t i = 0; i < bucket_cnt; ++i) {
			Node *p = buckets[i];
			while (p) {
				Node *nx = p->hnext;
				size_t ni = hasher(p->kv.first) % new_cnt;
				p->hnext = nb[ni];
				nb[ni] = p;
				p = nx;
			}
		}
		delete[] buckets;
		buckets = nb;
		bucket_cnt = new_cnt;
	}

	Node* find_node(const Key &key) const {
		if (bucket_cnt == 0) return nullptr;
		size_t idx = hasher(key) % bucket_cnt;
		Node *p = buckets[idx];
		while (p) {
			if (equaler(p->kv.first, key)) return p;
			p = p->hnext;
		}
		return nullptr;
	}

	void unlink_from_buckets(Node *node) {
		size_t idx = hasher(node->kv.first) % bucket_cnt;
		Node *p = buckets[idx], *pre = nullptr;
		while (p) {
			if (p == node) {
				if (pre) pre->hnext = p->hnext;
				else buckets[idx] = p->hnext;
				break;
			}
			pre = p; p = p->hnext;
		}
	}

public:
	linked_hashmap() { init_empty(); }
	linked_hashmap(const linked_hashmap &other) {
		init_empty(other.bucket_cnt ? other.bucket_cnt : init_bucket());
		for (Link *lp = other.root->next; lp != other.root; lp = lp->next) {
			Node *p = static_cast<Node*>(lp);
			Node *nnode = new Node(p->kv);
			relink_in_list_tail(nnode);
			buckets_insert(nnode);
			++sz;
		}
	}
 
	/**
	 * TODO assignment operator
	 */
	linked_hashmap & operator=(const linked_hashmap &other) {
		if (this == &other) return *this;
		destroy_all();
		init_empty(other.bucket_cnt ? other.bucket_cnt : init_bucket());
		for (Link *lp = other.root->next; lp != other.root; lp = lp->next) {
			Node *p = static_cast<Node*>(lp);
			Node *nnode = new Node(p->kv);
			relink_in_list_tail(nnode);
			buckets_insert(nnode);
			++sz;
		}
		return *this;
	}
 
	/**
	 * TODO Destructors
	 */
	~linked_hashmap() { destroy_all(); }
 
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) {
		Node *p = find_node(key);
		if (!p) throw index_out_of_bound();
		return p->kv.second;
	}
	const T & at(const Key &key) const {
		Node *p = find_node(key);
		if (!p) throw index_out_of_bound();
		return p->kv.second;
	}
 
	/**
	 * TODO
	 * access specified element 
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
		Node *p = find_node(key);
		if (p) return p->kv.second;
		rehash_if_needed();
		// in case rehash occurred above due to edge capacities
		if (bucket_cnt == 0) init_empty();
		Node *nnode = new Node(value_type(key, T()));
		relink_in_list_tail(nnode);
		buckets_insert(nnode);
		++sz;
		rehash_if_needed();
		return nnode->kv.second;
	}
 
	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
		Node *p = find_node(key);
		if (!p) throw index_out_of_bound();
		return p->kv.second;
	}
 
	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
		iterator it; it.owner = this; it.ptr = root->next; return it;
	}
	const_iterator cbegin() const {
		const_iterator it; it.owner = this; it.ptr = root->next; return it;
	}
 
	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {
		iterator it; it.owner = this; it.ptr = root; return it;
	}
	const_iterator cend() const {
		const_iterator it; it.owner = this; it.ptr = root; return it;
	}
 
	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const { return sz == 0; }
 
	/**
	 * returns the number of elements.
	 */
	size_t size() const { return sz; }
 
	/**
	 * clears the contents
	 */
    void clear() {
        // delete all nodes; keep buckets allocated and reset chains
        if (!root) return;
        Link *p = root->next;
        while (p != root) {
            Link *nxt = p->next;
            delete static_cast<Node*>(p);
            p = nxt;
        }
        root->next = root->prev = root;
        for (size_t i = 0; i < bucket_cnt; ++i) buckets[i] = nullptr;
        sz = 0;
    }
 
	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion), 
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
		Node *p = find_node(value.first);
		if (p) {
			iterator it; it.owner = this; it.ptr = p;
			return pair<iterator, bool>(it, false);
		}
		rehash_if_needed();
		Node *nnode = new Node(value);
		relink_in_list_tail(nnode);
		buckets_insert(nnode);
		++sz;
		rehash_if_needed();
		iterator it; it.owner = this; it.ptr = nnode;
		return pair<iterator, bool>(it, true);
	}
 
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
    void erase(iterator pos) {
        if (pos.owner != this || pos.ptr == nullptr || pos.ptr == root) throw invalid_iterator();
        Node *node = static_cast<Node*>(pos.ptr);
        // unlink from list
        node->prev->next = node->next;
        node->next->prev = node->prev;
        // unlink from buckets
        unlink_from_buckets(node);
		delete node;
		--sz;
	}
 
	/**
	 * Returns the number of elements with key 
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0 
	 *     since this container does not allow duplicates.
	 */
	size_t count(const Key &key) const { return find_node(key) ? 1 : 0; }
 
	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {
		iterator it; it.owner = this; it.ptr = find_node(key);
		if (it.ptr == nullptr) it.ptr = root;
		return it;
	}
	const_iterator find(const Key &key) const {
		const_iterator it; it.owner = this; it.ptr = find_node(key);
		if (it.ptr == nullptr) it.ptr = root;
		return it;
	}
};

}

#endif
