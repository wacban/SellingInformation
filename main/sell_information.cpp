#include "sell_information.h"
#include "../cut_and_choose/cut_and_choose.h"
#include "../timed_commitment/bit_utils.h"
#include "../timed_commitment/timed_commitment.h"
#include "../sha_commitment/sha_commitment.h"
#include "../common/aes.h"
#include "../common/log.h"
#include <vector>
#include <set>
#include <iostream>
#include <numeric>

using namespace CryptoPP;
using namespace std;

namespace sell_information {

using common::L;
using common::T;

void Seller::findRoots() {
	for(unsigned i = 0; i < common::L; ++i){
		array<Integer, 4> roots = square_root.all_square_roots(y[i]);
		r1[i] = roots[0];
		r2[i] = roots[1];
		if (common::rng().GenerateBit() == 1) {
			swap(r1[i], r2[i]);
		}
		if (r1[i] == -1) {
			// TODO random
		}
		if (r2[i] == -1) {
			// TODO random
		}
	}
}

void Seller::encryptRoots() {
	for(unsigned i = 0; i < L; ++i) {
		c1[i] = common::enc(single_seller.keys_commits[i].m, common::to_bytes(r1[i]));
		c2[i] = common::enc(single_seller.keys_commits[i].m, common::to_bytes(r2[i]));
	}
}

void Seller::setupCommit() {
	for(unsigned i = 0; i < L; ++i) {
		d1[i].m = c1[i];
		d2[i].m = c2[i];
	}
}

array<unsigned, L/2> Seller::acceptSubset(const std::array<unsigned, common::L/2>& indices, 
													const std::array<CryptoPP::Integer, common::L/2>& values) {
	array<unsigned, L/2> commitment_indices;	// which of c1, c2 shoudl be opened ?

	for(unsigned i = 0; i < L/2; ++i) {
		unsigned ind = indices[i];
		Integer val = values[i];
		if (val >= n/2) {
			throw ProtocolException("xi >= n/2");
		}
		if ((val*val) % n != y[ind]) {
			throw ProtocolException("xi is not a square root of yi");
		}
		commitment_indices[i] = 5; // TODO
		if (val == r1[ind]) {
			commitment_indices[i] = 1;
		}
		if (val == r2[ind]) {
			commitment_indices[i] = 2;
		}
	}
	return commitment_indices;
}

void Seller::accept_T1(const BitcoinTransaction& T1) {
	this->T1 = T1;
	// TODO verify that T2 spends money from T1
	// TODO verify hash
	// TODO wait for T1 to be final 
}

void Seller::accept_payment() {
	broadcast(this->single_seller.get_T2());
}

void Buyer::pickR(){
	this->r = Integer(common::rng(), 0, T-1).ConvertToLong();
}

void Buyer::pickSubset() {
	indices.resize(L);
	std::iota(indices.begin(), indices.end(), 0);
	common::rng().Shuffle(indices.begin(), indices.end());	
	indices.resize(L/2);
}

array<unsigned, L/2> Buyer::getSubsetIndices() {
	array<unsigned, L/2> res;
	copy(indices.begin(), indices.end(), res.begin());
	return res;
}

array<Integer, L/2> Buyer::getSubsetValues() {
	array<Integer, L/2> res;
	for(unsigned i = 0; i < L/2; ++i) {
		res[i] = x[indices[i]];
	}
	return res;
}

void Buyer::verifyRoot(unsigned ind, const vector<byte>& key, const vector<byte>& c) {
		vector<byte> x_encoded = common::dec(key, c);
		Integer x(x_encoded.data(), x_encoded.size());
		if (x != this->x[ind]) {
			throw ProtocolException("verifyRoot failed");
		}
}

void Buyer::make_payment() {
	broadcast(single_buyer.getT1());
}

void Buyer::start_brute_force() {
	
}

void Buyer::read_signature() {
	// TODO read the signature from bitcoin network
}

void Buyer::read_signature(const vector<byte>& signature) {
	setSignature(signature);
}

void Buyer::factorise() {
	auto keys = genKeys(signature);
	// TODO negate signature
	for(unsigned i = 0; i < L; ++i) {
		if (d1[i].m.empty() || d2[i].m.empty())
			continue;

		auto r1_bytes = common::dec(keys[i], d1[i].m);
		auto r2_bytes = common::dec(keys[i], d2[i].m);
		Integer r1(r1_bytes.data(), r1_bytes.size());
		Integer r2(r2_bytes.data(), r2_bytes.size());
		if (r1 < 0)
			r1 = -r1;
		if (r2 < 0)
			r2 = -r2;
		Integer p = GCD(n + r1 - r2, n);
		Integer q = GCD(r1 + r2, n);;
		if (p * q == n) {
			this->p = p;
			this->q = q;
			return;
		}
	}
	throw ProtocolException("Factorization failed!");
}

void SellInformationProtocol::init(Seller *seller, Buyer *buyer) {
	if (seller->get_n() != buyer->get_n()){
		throw ProtocolException("Not matching n");
	}

	if (seller->get_price() != buyer->get_price()){
		throw ProtocolException("Not matching price");
	}

	if (!verify(seller->get_address())){
		throw ProtocolException("Seller has invalid bitcoin address");
	}

	if (!verify(buyer->get_address())){
		throw ProtocolException("Buyer has invalid bitcoin address");
	}
}

void SellInformationProtocol::exec(Seller *seller, Buyer *buyer){
	Log("sell information protocol exec");
	SingleSellInformationProtocol single_sell_information;

	shared_signature::S shared_signature_s;
	shared_signature::B shared_signature_b(
		shared_signature_s.get_paillier_n(),
		shared_signature_s.get_paillier_g()
	);

	SingleSeller single_seller(shared_signature_s);
	SingleBuyer single_buyer(shared_signature_b);

	vector<SingleSeller> single_sellers(T, single_seller);
	vector<SingleBuyer> single_buyers(T, single_buyer);

	cut_and_choose::Prover<SingleSeller> prover;
	prover.v = single_sellers;
	cut_and_choose::Verifier<SingleBuyer> verifier;
	verifier.v = single_buyers;

	buyer->pickR();

	verifier.i = buyer->getR();

	cut_and_choose::cut_and_choose<SingleSellInformationProtocol, 
		SingleSeller,
		SingleBuyer,
		T > (&prover, &verifier);

	if (!verifier.res) {
		throw ProtocolException("Generating signatures and keys failed!");
	}

	Log("finished cut and choose");

	seller->setSingleSeller(prover.v[verifier.i]);
	buyer->setSingleBuyer(verifier.v[verifier.i]);

	buyer->genSquares();
	seller->acceptSquares(buyer->getSquares());
	seller->findRoots();
	seller->encryptRoots();
	seller->setupCommit();

	sha_commitment::ShaCommitment sha_commitment_protocol;

	for(unsigned i = 0; i < L; ++i) {
		sha_commitment_protocol.init(&seller->d1[i], &buyer->d1[i]);
		sha_commitment_protocol.init(&seller->d2[i], &buyer->d2[i]);

		sha_commitment_protocol.exec(&seller->d1[i], &buyer->d1[i]);
		sha_commitment_protocol.exec(&seller->d2[i], &buyer->d2[i]);
	}
	
	buyer->pickSubset();
	auto indices = buyer->getSubsetIndices();
	auto values = buyer->getSubsetValues();

	set<unsigned> indices_set(indices.begin(), indices.end());
	if (indices_set.size() != L/2) {
		Log(indices);
		throw ProtocolException("invalid indices size");
	}

	for(unsigned ind: indices_set) {
		if (ind >= L) {
			throw ProtocolException("invalid commitment_indices value");
		}
	}

	auto commitment_indices = seller->acceptSubset(indices, values);

	for(unsigned i = 0; i < L/2; ++i) {
		unsigned ind = indices[i];
		Integer val = values[i];
		unsigned commit_ind = commitment_indices[i];
		sha_commitment_protocol.open(&seller->single_seller.keys_commits[ind], &buyer->single_buyer.keys_commits[ind]);
		vector<byte> key = buyer->single_buyer.keys_commits[ind].m;
		vector<byte> c;
		switch (commit_ind) {
			case 1:
				sha_commitment_protocol.open(&seller->d1[ind], &buyer->d1[ind]);
				c = buyer->d1[ind].m;
				break;
			case 2:
				sha_commitment_protocol.open(&seller->d2[ind], &buyer->d2[ind]);
				c = buyer->d2[ind].m;
				break;
			default:
				throw ProtocolException("matching roots to values didn't work");
		}
		buyer->verifyRoot(ind, key, c);
	}

	for(unsigned ind = 0; ind < L; ++ind){
		if (indices_set.find(ind) == indices_set.end()) {
			sha_commitment_protocol.open(&seller->d1[ind], &buyer->d1[ind]);
			sha_commitment_protocol.open(&seller->d2[ind], &buyer->d2[ind]);
		}
	}

	buyer->make_payment();
	buyer->start_brute_force();

	seller->accept_T1(buyer->single_buyer.getT1());

	seller->accept_payment();

	buyer->read_signature(seller->single_seller.shared_signature_s.get_signature());
	// buyer->read_signature();

	buyer->factorise();
}

void SellInformationProtocol::open(Seller *seller, Buyer *buyer){

}

}
