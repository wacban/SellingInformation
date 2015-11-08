#include "aes.h"
#include <iostream>

using namespace CryptoPP;
using namespace std;

int main() {
	
	AutoSeededRandomPool rnd;

	byte key[AES::DEFAULT_KEYLENGTH];

	rnd.GenerateBlock(key, sizeof(key));

	for(int s = 0; s < 100000; s += 100){
		vector<byte> msg(s);
		rnd.GenerateBlock(msg.data(), msg.size());

		auto cipher = enc(key, msg);
		auto recovered = dec(key, cipher);

		if (recovered != msg) {
			cerr << "ERR" << endl;
			return;
		}	
	}
}
