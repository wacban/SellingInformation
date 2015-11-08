#include "square_root.h"

#include <iostream>

using namespace CryptoPP;
using namespace std;

int main(){

	Integer p = 17;
	Integer q = 19;

	SquareRoot sq(p, q);

	int r = 0;
	int nr = 0;

	for(int i = 0; i < p*q; ++i) {

		Integer a = (i*i) % (p*q);
		Integer root = sq.square_root(a);
		if ((root*root) % (p*q) != a){
			cout << "ERR: " << i << endl;
		}

		Integer root2 = sq(a);
		if ((root2*root2) % (p*q) != a){
			cout << "ERR: " << i << endl;
		}
	}
}
