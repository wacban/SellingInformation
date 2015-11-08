#include "aes.h"

#include <cryptopp/osrng.h>
#include <cryptopp/modes.h>

#include <string>

using namespace CryptoPP;
using namespace std;

vector<byte> enc(const array<byte, AES::DEFAULT_KEYLENGTH>& key, vector<byte> msg){
	string plain(msg.begin(), msg.end());
	string cipher;

	ECB_Mode< AES >::Encryption e;
	e.SetKey(key.data(), key.size());

	StringSource(plain, true, new StreamTransformationFilter(e, new StringSink(cipher)));

	return vector<byte>(cipher.begin(), cipher.end());
}

vector<byte> dec(const array<byte, AES::DEFAULT_KEYLENGTH>& key, vector<byte> msg) {

	string cipher(msg.begin(), msg.end());
	string plain;

	ECB_Mode< AES >::Decryption d;
	d.SetKey(key.data(), key.size());

	StringSource s(cipher, true, new StreamTransformationFilter(d, new StringSink(plain))); 

	return vector<byte>(plain.begin(), plain.end());
}


