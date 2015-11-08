#ifndef _SELL_INFORMATION_
#define _SELL_INFORMATION_

#include "../shared_signature/shared_sig.h"
#include "../bitcoin/bitcoin.h"
#include "../common/common.h"
#include "../protocol.h"
#include "../rsa/square_root.h"

#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>

namespace sell_information {

class Seller {
	CryptoPP::AutoSeededRandomPool *rng;
	CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> ec_parameters;

	// p*q = n, Buyer wants to buy p, q
	CryptoPP::Integer p;	
	CryptoPP::Integer q;

	unsigned price;

	BitcoinAddress bitcoin_address;

	SquareRoot square_root;

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
};

class Buyer {
	
	CryptoPP::AutoSeededRandomPool *rng;
	CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> ec_parameters;

	CryptoPP::Integer n;	// The rsa modulus - would like to buy primes p, q: p*q = n

	unsigned price;

	BitcoinAddress bitcoin_address;

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
};

class SellInformationProtocol : public Protocol<Seller, Buyer> {
	public:
	void init(Seller *, Buyer *);
	void exec(Seller *, Buyer *);
	void open(Seller *, Buyer *);
};

}

#endif

