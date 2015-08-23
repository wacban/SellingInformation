
#include "sha_commitment.h"

#include <iostream>
#include <cryptopp/osrng.h>

using namespace std;
using namespace CryptoPP;
using namespace sha_commitment;

const int T = 100000;

int main(){

	AutoSeededRandomPool rng;

	ShaCommitment shaCommitment;

	// test valid
	for(int i = 0; i < T; ++i){
		Sender s(&rng);
		Receiver r(&rng);

		Integer x(rng, (i % 1024) + 1);
		s.m = x;

		shaCommitment.init(&s, &r);
		shaCommitment.exec(&s, &r);

		if (!equal(s.com, s.com+COM_SIZE, r.com)){
			cerr << "ERR com not equal" << endl;
			return 1;
		}

		shaCommitment.open(&s, &r);
		if (s.m != r.m){
			cerr << "ERR s.m != r.m" << endl;
			return 1;
		}	
		if (r.getOpenVerified() == false){
			cerr << "ERR r.valid == false" << endl;
			return 1;
		}
		if (!equal(s.seed, s.seed+N, r.seed)){
			cerr << "ERR seed" << endl;
			return 1;
		}
	}

	for(int i = 0; i < T; ++i){
	
		Sender s(&rng);
		Receiver r(&rng);

		Integer x(rng, (i % 1024) + 1);
		s.m = x;

		shaCommitment.init(&s, &r);
		shaCommitment.exec(&s, &r);

		if (!equal(s.com, s.com+COM_SIZE, r.com)){
			cerr << "ERR com not equal" << endl;
			return 1;
		}

		// change m
		++s.m;

		shaCommitment.open(&s, &r);
		if (s.m != r.m){
			cerr << "ERR s.m != r.m" << endl;
			return 1;
		}	
		if (r.getOpenVerified() == true){
			cerr << "ERR r.valid == true" << endl;
			return 1;
		}
		if (!equal(s.seed, s.seed+N, r.seed)){
			cerr << "ERR seed" << endl;
			return 1;
		}
	}

	for(int i = 0; i < T; ++i){
	
		Sender s(&rng);
		Receiver r(&rng);

		Integer x(rng, (i % 1024) + 1);
		s.m = x;

		shaCommitment.init(&s, &r);
		shaCommitment.exec(&s, &r);

		if (!equal(s.com, s.com+COM_SIZE, r.com)){
			cerr << "ERR com not equal" << endl;
			return 1;
		}

		// change seed
		++s.seed[1];

		shaCommitment.open(&s, &r);
		if (s.m != r.m){
			cerr << "ERR s.m != r.m" << endl;
			return 1;
		}	
		if (r.getOpenVerified() == true){
			cerr << "ERR r.valid == true" << endl;
			return 1;
		}
		if (!equal(s.seed, s.seed+N, r.seed)){
			cerr << "ERR seed" << endl;
			return 1;
		}
	}

	cout << "OK" << endl;

	return 0;
}
