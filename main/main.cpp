#include "sell_information.h"
#include "signature_key.h"

#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>
#include <cryptopp/ecp.h>
#include <cryptopp/oids.h>
#include <cryptopp/eccrypto.h>

#include <iostream>
#include <vector>

using sell_information::Seller;
using sell_information::Buyer;
using sell_information::SellInformationProtocol;

using namespace CryptoPP;
using namespace std;

int main(){
	Integer p = 17;
	Integer q = 19;
	unsigned price = 20000;  // TODO

  vector<sell_information::BaseParty> v;

	Seller seller(p, q, price);
	Buyer buyer(p*q, price);

	SellInformationProtocol sell_information;

	sell_information.init(&seller, &buyer);
	sell_information.exec(&seller, &buyer);

	cout << buyer.p << "\t" << buyer.q << endl;

	return 0;
}

