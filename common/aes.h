#ifndef _AES_H_
#define _AES_H_

#include <cryptopp/aes.h>

#include <vector>
#include <array>

namespace common {
	
extern std::vector<byte> enc(const std::array<byte, CryptoPP::AES::DEFAULT_KEYLENGTH>& key, std::vector<byte> msg);

extern std::vector<byte> dec(const std::array<byte, CryptoPP::AES::DEFAULT_KEYLENGTH>& key, std::vector<byte> msg);

}

#endif

