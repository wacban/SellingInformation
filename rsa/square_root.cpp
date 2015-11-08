#include "square_root.h"

using namespace CryptoPP;

Integer SquareRoot::square_root(const Integer& a) {
	Integer xp = ModularSquareRoot(a, p);
	Integer xq = ModularSquareRoot(a, q);

	return CRT(xp, p, xq, q, u);
}

