#include <iostream>
#include <vector>
#include <iterator>

using namespace std;

namespace kpfp {
	namespace dlx {

		struct header;

		/**
		 * Node structure.
		 * Take a note, that for performance reasons, no constructor is given, thus
		 * all members take default values.
		 */
		struct node {
			node *L; /**< Node to the left */
			node *R; /**< Node to the right */
			node *U; /**< Node above */
			node *D; /**< Node below */
			header *C; /**< Link to the current column's header */
		};
	
		/**
		 * Header structure.
		 * Take a note, that for performance reasons, no constructor is given, thus
		 * all members take default values.
		 */
		struct header : public node {
			int S; /**< Size, i.e. number of 1's in this column */
			int N; /**< Name of current header */
		};
	}

	/**
	 * Solves generalized exact cover problem using DLX algorithm.
	 * Single-threaded.
	 *
	 * @tparam Derived Static polymorphism via CRTP. Used to return results
	 * 		   to user-defined functions.
	 */
	template <class Derived>
	class dlxSolver {
	protected:
		std::vector<dlx::header> h; /**< Headers. h[0] is master header. */
		std::vector<dlx::node*> O; /**< Result vector. */

		/**
		 * Cover column c.
		 *
		 * @param c Header
		 */
		void cover(dlx::header *c);

		/**
		 * Uncover column c.
		 *
		 * @param c Header
		 */
		void uncover(dlx::header*);
	public:
		/**
		 * Constructor.
		 */
		dlxSolver() {
			h.resize(1); // create master header
		}

		/**
		 * Copy constructor.
		 *
		 * @param f Foreign object to be copied.
		 */
		dlxSolver(const dlxSolver &f);

		/**
		 * Main algorithm
		 *
		 * @param k Depth of a search.
		 * @return Returns a reference to results.
		 */
		void search(unsigned int k=0);

		/**
		 * Get results.
		 *
		 * @return Reference to results.
		 */
		const std::vector<dlx::node*> getResults() { return O; }

		/**
		 * Set column count.
		 * Reserves required capacity for output and columns.
		 *
		 * @param p Primary columns
		 * @param s Secondary columns
		 */
		void setColumnNumber(unsigned int p, unsigned int s=0);
		
		/**
		 * Fills the search matrix.
		 * This method doesn't check assumptions. Each row shall contain numbers in range
		 * [1; p+s] (in ascending order) that indicate, in which columns are 1s.
		 *
		 * @attention setColumnNumber must be called first.
		 * 
		 * @tparam InputIterator Iterator type (as described in SGI's STL documentation).
		 * @param it Iterator pointing to integers (each x: 0 < x <= p+s) in ascending order.
		 * @param end Iterator's end point.
		 * @see setColumnNumber
		 */
		template <class InputIterator>
		void addRow(InputIterator it, InputIterator end);

		/**
		 * Interface to user-defined function.
		 * Uses CRTP to achieve static-polymorphism. Calls user-defined method of the same
		 * name.
		 *
		 * To get results, user should:
		 *  - for every selected row i = 0, 1, ..., k-1
		 *  - for every kpfp::dlx::node *n = O[i], O[i]->R, O[i]->R->R... (until n==O[i] again).
		 *  - get column number with n->C->N
		 *
		 * @param k Rows that cover the search-space.
		 */
		void solution(unsigned int k) {
			static_cast<Derived*>(this)->solution(k);
		}
	};
}


template <class Derived>
template <class InputIterator>
void kpfp::dlxSolver<Derived>::addRow(InputIterator it, InputIterator end) {
	dlx::node s; /* sentry */
	s.R = &s;
	dlx::node *l = &s; /* node to the left */

	for(; it!=end; ++it) {
		dlx::node *n = new dlx::node();
		int hN = *it;
		++h[hN].S;
		n->C = &h[hN];
		n->U = h[hN].U;
		n->D = &h[hN];
		n->L = l;
		l->R = n;
		h[hN].U->D = n;
		h[hN].U = n;
		l = n;
	}
	l->R = s.R;
	s.R->L = l;
}

template <class Derived>
void kpfp::dlxSolver<Derived>::setColumnNumber(unsigned int p, unsigned int s) {
	O.resize(p+s);
	h.resize(p+s+1);

	h[0].R = &h[0];
	
	unsigned int i = 1;
	for(; i<=p; ++i) {
		h[i].L = &h[i-1];
		h[i].L->R = &h[i];
		h[i].U = h[i].D = &h[i];
		h[i].N = i;
	}
	h[p].R = &h[0];
	h[0].L = &h[p];

	unsigned int sum = p+s;
	for(; i<=sum; ++i) {
		h[i].L = h[i].R = h[i].U = h[i].D = &h[i];
		h[i].N = i;
	}
}

template <class Derived>
void kpfp::dlxSolver<Derived>::search(unsigned int k) {
	dlx::header &m = h[0];
	if(m.R == &m && m.L == &m) { // termination condition
		solution(k);
		return;
	}
	// select column (to minimize branching factor)
	dlx::header *c = static_cast<dlx::header*>(m.R);
	int s = c->S;
	for(dlx::node *j=m.R; j!=static_cast<dlx::node*>(&m); j=j->R) {
		if(static_cast<dlx::header*>(j)->S < s) {
			s = static_cast<dlx::header*>(j)->S;
			c = static_cast<dlx::header*>(j);
		}
	}
	cover(c); // cover column c
	for(dlx::node *r=c->D; r!=c; r=r->D) { // for each row...
		O[k] = r;
		for(dlx::node *j=r->R; j!=r; j=j->R) // for each column of this node...
			cover(j->C);
		search(k+1);
		r = O[k];
		c = r->C;
		for(dlx::node *j=r->L; j!=r; j=j->L)
			uncover(j->C);
	}
	uncover(c); //uncover column c
}

template <class Derived>
void kpfp::dlxSolver<Derived>::cover(dlx::header *c) {
	c->R->L = c->L;
	c->L->R = c->R;
	for(dlx::node *i=c->D; i!=static_cast<dlx::node*>(c); i=i->D) {
		for(dlx::node *j=i->R; j!=i; j=j->R) {
			j->D->U = j->U;
			j->U->D = j->D;
			--(j->C->S);
		}
	}
}

template <class Derived>
void kpfp::dlxSolver<Derived>::uncover(dlx::header *c) {
	for(dlx::node *i=c->U; i!=static_cast<dlx::node*>(c); i=i->U) {
		for(dlx::node *j=i->L; j!=i; j=j->L) {
			++(j->C->S);
			j->D->U = j;
			j->U->D = j;
		}
	}
	c->L->R = c;
	c->R->L = c;
}

