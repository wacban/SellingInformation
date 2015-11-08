#ifndef _SELL_INFORMATION_
#define _SELL_INFORMATION_

#include "../shared_signature/shared_sig.h"
#include "../bitcoin/bitcoin.h"
#include "../common/common.h"
#include "../protocol.h"
#include "../common/square_root.h"

#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>

namespace sell_information {

const unsigned T = 10;
const unsigned K = 10;	// TODO
const unsigned L = 10;	// TODO

class Seller {
	CryptoPP::AutoSeededRandomPool *rng;
	CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> ec_parameters;

	// p*q = n, Buyer wants to buy p, q
	CryptoPP::Integer p;	
	CryptoPP::Integer q;

	unsigned price;

	BitcoinAddress bitcoin_address;

	common::SquareRoot square_root;

	std::array<CryptoPP::Integer, L> y;
	std::array<CryptoPP::Integer, L> r1;
	std::array<CryptoPP::Integer, L> r2;

	public:
	Seller(	CryptoPP::AutoSeededRandomPool *rng, 
					CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> &ec_parameters,
					CryptoPP::Integer p, 
					CryptoPP::Integer q, 
					unsigned price, 
					BitcoinAddress bitcoin_address):
		rng(rng), ec_parameters(ec_parameters), p(p), q(q), price(price), bitcoin_address(bitcoin_address), square_root(p, q) {}

	CryptoPP::AutoSeededRandomPool *getRng(){
		return rng;
	}
	CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> getEcParameters(){
		return ec_parameters;
	}

	CryptoPP::Integer get_n(){
		return p*q;
	}

	unsigned get_price(){
		return price;
	}
	
	BitcoinAddress get_address(){
		return bitcoin_address;
	}

	void acceptSquares(const std::array<CryptoPP::Integer, L>& y){
		this->y = y;
	}

	void findRoots();
	
	void encryptRoots();
};

class Buyer {
	
	CryptoPP::AutoSeededRandomPool *rng;
	CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> ec_parameters;

	CryptoPP::Integer n;	// The rsa modulus - would like to buy primes p, q: p*q = n

	unsigned price;

	BitcoinAddress bitcoin_address;

	std::array<CryptoPP::Integer, L> x;
	std::array<CryptoPP::Integer, L> y;

	int r;

	public:
	Buyer(CryptoPP::AutoSeededRandomPool *rng, 
				CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> &ec_parameters,
				CryptoPP::Integer n, 
				unsigned price,
				BitcoinAddress bitcoin_address):
		rng(rng), ec_parameters(ec_parameters), n(n), price(price), bitcoin_address(bitcoin_address) {}

	CryptoPP::AutoSeededRandomPool *getRng(){
		return rng;
	}

	CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> getEcParameters(){
		return ec_parameters;
	}

	CryptoPP::Integer get_n(){
		return n;
	}

	unsigned get_price(){
		return price;
	}

	BitcoinAddress get_address(){
		return bitcoin_address;
	}

	void pickR();

	int getR(){
		return r;
	}

	void genSquares(){
		for(unsigned i = 0; i < L; ++i) {
			x[i] = CryptoPP::Integer(common::rng, 0, n/2);
			y[i] = (x[i] * x[i]) % n;
		}
	}

	std::array<CryptoPP::Integer, L> getSquares(){
		return y;
	}
};

class SellInformationProtocol : public Protocol<Seller, Buyer> {
	public:
	void init(Seller *, Buyer *);
	void exec(Seller *, Buyer *);
	void open(Seller *, Buyer *);
};

}

#endif

