#ifndef _COMMON_H_
#define _COMMON_H_

#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>

#include <array>
#include <vector>

namespace common {

extern CryptoPP::AutoSeededRandomPool rng;
extern CryptoPP::SHA256 sha256;

typedef std::array<byte, 32> TSHA256Digest;

template <typename T>
std::vector<T> concatenate(const std::vector<T>& x, const std::vector<T>& y) {
	std::vector<T> res;

	res.insert(res.end(), x.begin(), x.end());
	res.insert(res.end(), y.begin(), y.end());
	return res;
}

}

#endif 
