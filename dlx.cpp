#include <cstdio>
#include <iostream>
#include <cstring>
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
	 */
	class dlxSolver {
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
		void search(int k=0);

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
		void setColumns(unsigned int p, unsigned int s=0);
		
		/**
		 * Fills the search matrix.
		 * This method doesn't check assumptions. Each row shall contain numbers in range
		 * [1; p+s] (in ascending order) that indicate, in which columns are 1s.
		 *
		 * @attention setColumns must be called first.
		 * 
		 * @tparam InputIterator Iterator type (as described in SGI's STL documentation).
		 * @param it Iterator pointing to integers (each x: 0 < x <= p+s) in ascending order.
		 * @param end Iterator's end point.
		 * @see setColumns
		 */
		template <class InputIterator>
		void addRow(InputIterator it, InputIterator end);
	};
}


int main() {
	int cols;
	int rows;
	int currCols;
	kpfp::dlxSolver a;
	std::cin >> cols >> rows;
	a.setColumns(cols);
	while(rows--) {
		std::cin >> currCols;
		std::vector<int> cols;
		int tmp;
		while(currCols--) {
			cin >> tmp;
			cols.push_back(tmp);
		}
		a.addRow(cols.begin(), cols.end());
	}
	a.search(0);

	return 0;
}

template <class InputIterator>
void kpfp::dlxSolver::addRow(InputIterator it, InputIterator end) {
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

void kpfp::dlxSolver::setColumns(unsigned int p, unsigned int s) {
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

void kpfp::dlxSolver::search(int k) {
	/*dbg*///printf("search(%d)\n", k);
	dlx::header &m = h[0];
	// termination condition
	if(m.R == &m && m.L == &m) {
		/*dbg*///printf("\t* solution!\n\t\t");
		std::cout << "Solution:\n";
		for(int i=0; i<k; ++i) {
			std::cout << O[i]->C->N;
			/*dbg*///printf("%d", O[i]->C->N);
			for(dlx::node *n=O[i]->R; n!=O[i]; n=n->R)
				std::cout << " " << n->C->N;
				/*dbg*///printf(",%d", n->C->N);
			/*dbg*///printf("\n\t\t");
			std::cout << "\n";
		}
		/*dbg*///printf("\n");
		/*dbg*///printf("search(%d) DONE\n",k);
		return;
	}
	// select column (to minimize branching factor)
	/*dbg*///printf("\t* select time\n");
	/*dbg*///printf("\t* HEADERS=%d(%d)", h[0].N, h[0].S);
	//for(dlx::header *j=static_cast<dlx::header*>(h[0].R); j!=&h[0]; j=static_cast<dlx::header*>(j->R))
		/*dbg*///printf(", %d(%d)",j->N,j->S);
	/*dbg*///printf("\n");
	dlx::header *c = static_cast<dlx::header*>(m.R);
	int s = c->S;
	for(dlx::node *j=m.R; j!=static_cast<dlx::node*>(&m); j=j->R) {
		if(static_cast<dlx::header*>(j)->S < s) {
			s = static_cast<dlx::header*>(j)->S;
			c = static_cast<dlx::header*>(j);
		}
	}
	/*dbg*///printf("\t* selected %d size=%d\n",c->N, c->S);
	// cover column c
	cover(c);
	// for each row...
	for(dlx::node *r=c->D; r!=c; r=r->D) {
		/*dbg*///printf("\t* COVER TIME(%d)\n",k);
		/*dbg*///printf("\t* O[%d]=%d",k,r->C->N);
		//for(dlx::node *j=r->R; j!=r; j=j->R)
			/*dbg*///printf(",%d",j->C->N);
		/*dbg*///printf("\n");
		/*dbg*///printf("\t* HEADERS=%d", h[0].N);
		//for(dlx::header *j=static_cast<dlx::header*>(h[0].R); j!=&h[0]; j=static_cast<dlx::header*>(j->R))
			/*dbg*///printf(",%d",j->N);
		/*dbg*///printf("\n");
		O[k] = r;
		// for each column of this node...
		for(dlx::node *j=r->R; j!=r; j=j->R)
			cover(j->C);
		search(k+1);
		r = O[k];
		c = r->C;
		for(dlx::node *j=r->L; j!=r; j=j->L)
			uncover(j->C);
	}
	/*dbg*///printf("\t* No luck...(%d)\n",k);
	//uncover column c
	uncover(c);
	/*dbg*///printf("search(%d) DONE\n",k);
}

void kpfp::dlxSolver::cover(dlx::header *c) {
	/*dbg*///printf("\t* cover(header:%d)\n",c->N);
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

void kpfp::dlxSolver::uncover(dlx::header *c) {
	/*dbg*///printf("\t* uncover(header:%d)\n",c->N);
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

