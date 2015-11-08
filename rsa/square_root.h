#ifndef _SQUARE_ROOT_H_
#define _SQUARE_ROOT_H_

#include "cryptopp/integer.h"
#include "cryptopp/nbtheory.h"

class SquareRoot {
	CryptoPP::Integer p;
	CryptoPP::Integer q;

	CryptoPP::Integer u;

	public:
	SquareRoot(CryptoPP::Integer p, CryptoPP::Integer q): p(p), q(q) {
		u = p.InverseMod(q);
	}

	CryptoPP::Integer square_root(const CryptoPP::Integer& a);
	CryptoPP::Integer operator()(const CryptoPP::Integer& a){
		return square_root(a);
	}
};

#endif 
