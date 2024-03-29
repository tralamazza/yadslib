/*
 * bst.h
 *
 *  Created on: Aug 20, 2011
 *      Author: Daniel Tralamazza
 *
 *  Binary Search Tree with
 *  - parent pointers
 *  - no duplicates (acts like a set)
 */

#ifndef BST_H_
#define BST_H_

#include <iostream>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>

namespace yadslib {

template <typename _Key, typename _Alloc = std::allocator<_Key> >
class binary_search_tree {
private:
	// trivial node class
	struct node {
		typedef node* pointer;
		typedef node& reference;
		typedef const reference const_reference;

		_Key data;
		pointer parent;
		pointer edge[2]; // 0 -> left, 1 -> right
		pointer left() const { return edge[0]; }
		void left(pointer n) { edge[0] = n; }
		pointer right() const { return edge[1]; }
		bool operator==(const_reference other) const { return data == other.data; }
		bool operator!=(const_reference other) const { return data != other.data; }
		bool operator<(const_reference other) const { return data < other.data; }
		static pointer left_most(pointer pnode) {
			while (pnode->left())
				pnode = pnode->left();
			return pnode;
		}
		static pointer right_most(pointer pnode) {
			while (pnode->right())
				pnode = pnode->right();
			return pnode;
		}
	};

	// in order tree traversal
	class inorder_traversal {
	private:
		node* io_node;

	protected:
		inorder_traversal(node* _node, bool _from_start) : io_node(_node) {
			if (io_node && _from_start) {
				io_node = node::left_most(io_node);
			}
		}

		node* current() const { return io_node; }

		node* successor() {
			if (io_node == NULL)
				return NULL; // trivial case
			if (io_node->right()) {
				// node has a right kid
				io_node = node::left_most(io_node->right()); // go down right and
				return io_node; // get the left most
			} else { // right kid is null
				// while io_node is not root and is different from its parent left edge
				while ( io_node->parent != NULL && (io_node != io_node->parent->left()) ) {
					io_node = io_node->parent; // go up
				}
				io_node = io_node->parent;
				return io_node;
			}
		}

		node* predecessor() { return NULL; } // TODO for bidirectional iterator
	};

public:
	typedef _Alloc allocator_type;
	typedef typename _Alloc::value_type value_type;
	typedef typename _Alloc::reference reference;
	typedef typename _Alloc::const_reference const_reference;
	typedef typename _Alloc::pointer pointer;
	typedef typename _Alloc::const_pointer const_pointer;
	typedef typename _Alloc::size_type size_type;


	template <class TraversalPolicy>
	class node_iterator : public TraversalPolicy, std::iterator<std::forward_iterator_tag, _Key> {
	public:
		//node_iterator(const node_iterator& _it) : TraversalPolicy(_it.pnode, false), pnode(_it.pnode) {	}
		node_iterator(node* _node, bool _from_start) : TraversalPolicy(_node, _from_start) {
			pnode = this->current();
		}
		bool operator==(const node_iterator& other) {
			return (!pnode && !other.pnode) || (pnode && other.pnode && (*pnode) == (*other.pnode));
		}
		bool operator!=(const node_iterator& other) { return !(*this == other); }
		node_iterator& operator++() { pnode = this->successor(); return *this; }
		node_iterator& operator--() { pnode = this->predecessor(); return *this; }
		typename node_iterator::reference operator*() const { return pnode->data; }
		typename node_iterator::pointer operator->() const { return &pnode->data; }
	private:
		node* pnode;
	};

	typedef node_iterator<inorder_traversal> iterator;

	binary_search_tree() : root(NULL), m_size(0) { }
	~binary_search_tree() { clear(); }

	size_t size() const { return m_size; }

	bool empty() const { return m_size == 0; }

	void clear() { if (root) destroy_node_descendants(root); }

	std::pair<iterator, bool> insert(const_reference x) {
		// special case root is null
		if (root == NULL) {
			root = create_node(x, NULL); // create a node without a parent
			return std::make_pair(begin(), true);
		}
		int dir;
		node* pn = root, *n;
		for (;;) {
			// we found the node, just return it
			if (x == pn->data)
				return std::make_pair(iterator(pn, false), false);
			// which direction (edge) to take
			dir = (x < pn->data) ? 0 : 1;
			// grab the kid
			n = pn->edge[dir];
			// if the kid is empty, we can insert here
			if (n == NULL) {
				// create node with parent node pn
				n = create_node(x, pn);
				// pn points to the newly created node n
				pn->edge[dir] = n;
				return std::make_pair(iterator(n, false), true);
			} else
				pn = n; // continue searching
		}
		// should never reach
		return std::make_pair(end(), false);
	}

	template<typename _InputIterator>
	void insert(_InputIterator first, _InputIterator last) {
		for (; first != last; ++first)
			insert(*first);
	}

	size_t erase(const_reference x) {
		int dir;
		node* n = find_node(x, dir);
		if (n == NULL)
			return 0; // not found
		node* pn = n->parent; // n's parent
		node* rn = n->right(); // n's right kid
		if (rn == NULL) {
			// n has no right kid
			if (n->left()) // if n's left is valid
				n->left()->parent = pn; // set its parent to p
			if (pn)
				pn->edge[dir] = n->left(); // p links to n's lef
			else
				root = n->left();
		} else {
			if (rn->left() == NULL) {
				// n's right kid has no left kid
				rn->left(n->left()); // right kid inherit n's left
				if (rn->left()) // set our new left kid rn as parent
					rn->left()->parent = rn;
				rn->parent = pn; // set right kid's parent to p
				if (pn)
					pn->edge[dir] = rn; // p links to right kid
				else
					root = rn;
			} else {
				// n's right kid has a left kid
				node* ln = node::left_most(rn->left()); // find left most descendant
				// reassign ln's right kid
				ln->parent->left(ln->right()); // ln's parent left points to ln's right
				if (ln->right()) // if ln has a right kid
					ln->right()->parent = ln->parent; // his parent is now ln's parent
				std::swap(n, ln); // swap pointers (to destroy ln as n)
				std::swap(n->data, ln->data); // swap data
			}
		}
		destroy_node(n); // destroy n
		return 1;
	}

	template<typename _InputIterator>
	void erase(_InputIterator first, _InputIterator last) {
		for (; first != last; ++first)
			erase(*first);
	}

	size_t count(const_reference x) const {
		int dir;
		return find_node(x, dir) ? 1 : 0;
	}

	iterator find(const_reference x) const {
		int dir;
		return iterator(find_node(x, dir), false);
	}

	iterator begin() const { return iterator(root, true); }
	iterator end() const { return iterator(NULL, false); }

	const_reference min() const {
		node* n = node::left_most(root);
		return n->data;
	}

	const_reference max() const {
		node* n = node::right_most(root);
		return n->data;
	}

private:
	node* root;

	size_t m_size;

	// rebind to allocate nodes instead of _Key
	typename allocator_type::template rebind<node>::other node_alloc;

	// post process this node down, destroying also all descendants
	void destroy_node_descendants(node* pnode) {
		if (pnode->left())
			destroy_node_descendants(pnode->left());
		if (pnode->right())
			destroy_node_descendants(pnode->right());
		if (pnode->parent) {
			pnode->parent->edge[(*pnode < *pnode->parent) ? 0 : 1] = NULL;
		}
		destroy_node(pnode);
	}

	// deallocates pnode and decrement size
	void destroy_node(node* pnode) {
		if (--m_size == 0 && pnode->parent == NULL)
			root = NULL; // special case
		node_alloc.deallocate(pnode, 1);
	}

	// allocates a new node and increment size
	node* create_node(const_reference x, node* parent) {
		node* nn = node_alloc.allocate(1);
		nn->data = x;
		nn->parent = parent;
		++m_size;
		return nn;
	}

	/* find a node by its value, also return the direction
	 from which we got there (from a parents perspective) */
	node* find_node(const_reference x, int &dir) const {
		dir = 0; // default left (see erase)
		node* n = root;
		while (n && x != n->data) {
			dir = (x < n->data) ? 0 : 1;
			n = n->edge[dir];
		}
		return n;
	}
};

}; // namespace

#endif /* BST_H_ */
