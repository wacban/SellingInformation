#ifndef _SHA_COMMITMENT_H_
#define _SHA_COMMITMENT_H_

#include "../protocol.h"

#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>

namespace sha_commitment {

const unsigned N = 20; // security parameter in bytes

const unsigned COM_SIZE = 256/8;

class Sender {

	public:

	Sender(CryptoPP::AutoSeededRandomPool *rng) : rng(rng) {}

	CryptoPP::AutoSeededRandomPool *rng;
	CryptoPP::Integer m;
	byte seed[N];
	byte com[COM_SIZE];

};

class Receiver {

	bool open_verified;

	public:

	Receiver(CryptoPP::AutoSeededRandomPool *rng) : rng(rng) {}

	CryptoPP::AutoSeededRandomPool *rng;
	CryptoPP::Integer m;
	byte seed[N];
	byte com[COM_SIZE];

	void setOpenVerified(bool val){
		open_verified = val;
	}

	bool getOpenVerified(){
		return open_verified;
	}
};

class ShaCommitment : public Protocol<Sender, Receiver> {
	public:
	void init(Sender *, Receiver *);
	void exec(Sender *, Receiver *);
	void open(Sender *, Receiver *);
};

}

#endif

