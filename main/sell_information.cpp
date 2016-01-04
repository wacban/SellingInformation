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
#include <thread>
#include <unistd.h>

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

void Seller::accept_T1(const Transaction& T1) {
	this->T1 = T1;

  bool res = single_seller.base_party.client.verifyTransaction(
    T1,
    common::integer2string(single_seller.shared_signature_s.get_Q().x),
    common::integer2string(single_seller.shared_signature_s.get_Q().y),
    price
  );

  if (!res) {
    throw ProtocolException("verify T1 failed!");
  } else {
    Log("verify transaction worked!");
  }

  
  /* TODO verify T2 */

  res = single_seller.base_party.client.waitForTransaction(T1, 1); // TODO better security
  if (!res) {
    throw ProtocolException("wait for t1 failed!");
  } else {
    Log("wait for t1 worked!");
  }
}

void Seller::accept_payment() {
  bool res = single_seller.base_party.client.broadcastTransaction(
    single_seller.getT2()
  );
   
  if (!res) {
    throw ProtocolException("accept payment failed!");
  } else {
    Log("accept payment worked!");
  }

	// broadcast(this->single_seller.get_T2());
  // TODO
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
  bool res = single_buyer.base_party.client.broadcastTransaction(
    single_buyer.getT1()
  );
  if (!res) {
    throw ProtocolException("make payment failed");
  } else {
    Log("payment succesfull");
  }
}

void Buyer::solve_time_lock(atomic_bool &finished) {
  Log("solve time lock sleeping forever");
	while(true) {
	  sleep(1);
	}
	
	this->single_buyer.timed_commitment_receiver.force_open_smart();
	finished = true;	
}

void Buyer::get_signature(atomic_bool &finished) {
	/*if (common::rng().GenerateBit() == 1) {
		Log("get singature sleeping");
		while(true) {
			sleep(1);
		}
	}
  */
  bool res;
	Log("get signature not sleeping");
  res = single_buyer.base_party.client.waitForTransactionByOutput(single_buyer.getSellerAddress(), 1); // TODO better security
  if (!res) {
    throw ProtocolException("wait for transaction by output failed");
  }

  bitcoin_utils::IntegerPair signature;
  single_buyer.base_party.client.getSignature(signature, single_buyer.getSellerAddress());

  Integer r = Integer((byte*) signature.r.data(), signature.r.size());
  Integer s = Integer((byte*) signature.s.data(), signature.s.size());
  this->signature = shared_signature::encode_signature(r, s);
  Log("-------------------------------------------");
  Log("Signature from btc");
  Log(common::string2hex(common::v2string(this->signature)));
  Log("-- r s");
  Log(r);
  Log(s);
  Log("-------------------------------------------");
	
	factorise();
	finished = true;
}

void Buyer::wait_for_signature_or_time_lock() {
	atomic_bool signature_finished(false);
	atomic_bool time_lock_finished(false);

	thread solve_time_lock(&Buyer::solve_time_lock, this, ref(time_lock_finished));
	thread get_signature(&Buyer::get_signature, this, ref(signature_finished));
	
	Log("wait_for_signature_or_time_lock waiting");
	while(!signature_finished && !time_lock_finished) {
		sleep(1);
	}
	Log("wait_for_signature_or_time_lock finished");

	if (signature_finished) {
		get_signature.join();
		solve_time_lock.detach();
		Log("signature finished");
	} else {
		Log("time lock finished");
		get_signature.detach();
		solve_time_lock.join();
	}
}

void Buyer::factorise() {
  vector<byte> sig(signature.begin(), signature.end());
	auto keys = genKeys(sig);
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

	/*
  if (!verify(seller->get_address())){
		throw ProtocolException("Seller has invalid bitcoin address");
	}
  */

  /*
	if (!verify(buyer->get_address())){
		throw ProtocolException("Buyer has invalid bitcoin address");
	}
  */
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
    // TODO 
		throw ProtocolException("Generating signatures and keys failed!");
	}

	Log("finished cut and choose");

	seller->setSingleSeller(prover.v[verifier.i]);
  Log("-------------------------------------------");
  Log("Generated signature:");
  Log(
    common::string2hex(
      common::v2string(
        seller->single_seller.shared_signature_s.get_signature()
      )
    )
  );
  Log(seller->single_seller.shared_signature_s.get_r());
  Log(seller->single_seller.shared_signature_s.get_s());
  Log("-------------------------------------------");

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

	seller->accept_T1(buyer->single_buyer.getT1());

	seller->accept_payment();	 // last function for seller
 
 	// buyer->set_signature(seller->single_seller.shared_signature_s.get_signature()); // for testing TODO remove
	buyer->wait_for_signature_or_time_lock(); // last function for buyer
}

void SellInformationProtocol::open(Seller *seller, Buyer *buyer){

}

}
