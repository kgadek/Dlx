#include "dlx.hpp"
#include <iostream>

using namespace std;
using namespace kpfp;

struct Printer : public dlxSolver<Printer> {
	void solution(unsigned int k) {
		for(int i=0; i<k; ++i) {
			cout << "(" << O[i]->C->N;
			for(dlx::node *n=O[i]->R; n!=O[i]; n=n->R)
				cout << " " << n->C->N;
			cout << ") ";
		}
		cout << "\n";
	}
};


int main() {
	int cols;
	int rows;
	int currCols;
	Printer a;
	cin >> cols >> rows;
	a.setColumnNumber(cols);
	while(rows--) {
		cin >> currCols;
		vector<int> cols;
		int tmp;
		while(currCols--) {
			cin >> tmp;
			cols.push_back(tmp);
		}
		a.addRow(cols.begin(), cols.end());
	}
	a.search();

	return 0;
}

