
#include "sell_information.h"
#include "../cut_and_choose/cut_and_choose.h"
#include "../timed_commitment/bit_utils.h"
#include "../timed_commitment/timed_commitment.h"
#include "../sha_commitment/sha_commitment.h"
#include <vector>
#include <iostream>

using namespace CryptoPP;
using namespace std;

const unsigned T = 10;
const unsigned K = 10;	// TODO
const unsigned L = 10;	// TODO

namespace sell_information {

static array<common::TSHA256Digest, L> genKeys(const vector<byte>& signature) {
	array<common::TSHA256Digest, L> keys;
	for(unsigned i = 0; i < L; ++i) {
		Integer I(i);
		vector<byte> i_encoded(I.MinEncodedSize());
		I.Encode(i_encoded.data(), i_encoded.size());

		vector<byte> tmp = common::concatenate(signature, i_encoded);
		common::sha256.CalculateDigest(keys[i].data(), tmp.data(), tmp.size());

		// keys_commits[i].m.assign(keys[i].begin(), keys[i].end()); // TODO
	}
	// signature_commit.m = signature; TODO
	return keys;
}

class SingleSeller {
	AutoSeededRandomPool *rng;
	DL_GroupParameters_EC<ECP> ec_parameters;

	BitcoinTransaction T2;

	array<common::TSHA256Digest, L> keys;

	public:
	shared_signature::S shared_signature_s;
	timed_commitment::Commiter timed_commitment_commiter;

	array<sha_commitment::Sender, L> keys_commits;
	sha_commitment::Sender signature_commit;

	SingleSeller(	CryptoPP::AutoSeededRandomPool *rng, 
								CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> ec_parameters,
								shared_signature::S shared_signature_s):
			rng(rng), ec_parameters(ec_parameters), shared_signature_s(shared_signature_s), 
			timed_commitment_commiter(rng), keys_commits() {}
					
	void setT2(BitcoinTransaction T2){
		this->T2 = T2;
	}
	
	void cheat(){
		// for testing
		throw ProtocolException("Cheating not implemented!");
	}

	AutoSeededRandomPool *getRng(){
		return rng;
	}

	void genKeysAndSetupCommits(){
		vector<byte> signature = shared_signature_s.get_signature();

		keys = genKeys(signature);
		for(unsigned i = 0; i < L; ++i) {
			keys_commits[i].m.assign(keys[i].begin(), keys[i].end());
		}
		signature_commit.m = signature;
	}
};

class SingleBuyer {
	
	AutoSeededRandomPool *rng;
	DL_GroupParameters_EC<ECP> ec_parameters;

	BitcoinTransaction T1;
	BitcoinTransaction T2;

	bool open_verified;

	public:
	shared_signature::B shared_signature_b;
	timed_commitment::Receiver timed_commitment_receiver;

	array<sha_commitment::Receiver, L> keys_commits;
	sha_commitment::Receiver signature_commit;

	SingleBuyer(AutoSeededRandomPool *rng, 
							DL_GroupParameters_EC<ECP> ec_parameters,
							shared_signature::B shared_signature_b):
			rng(rng), ec_parameters(ec_parameters), open_verified(false), shared_signature_b(shared_signature_b), timed_commitment_receiver(rng) {}
	
	AutoSeededRandomPool* getRng(){
		return rng;
	}

	void createT1(){
		/* TODO */
	}

	void createT2(){
		/* TODO */
	}
	
	BitcoinTransaction getT2(){
		return T2;
	}

	void setOpenVerified(bool open_verified){
		this->open_verified = open_verified;
	}

	bool getOpenVerified() {
		return open_verified;
	}

	void verifyKeys() {
		vector<byte> signature = signature_commit.m;
		array<common::TSHA256Digest, L> exp_keys = genKeys(signature);
		for(unsigned i = 0; i < L; ++i){
			if (!equal(keys_commits[i].m.begin(), keys_commits[i].m.end(), exp_keys[i].begin())) {
				throw ProtocolException("Invalid key");
			}
		}
	}
};

class SingleSellInformationProtocol : public Protocol<SingleSeller, SingleBuyer> {
	public:
	void init(SingleSeller *, SingleBuyer *);
	void exec(SingleSeller *, SingleBuyer *);
	void open(SingleSeller *, SingleBuyer *);
};


void SingleSellInformationProtocol::init(SingleSeller *, SingleBuyer *) {
	
}

void SingleSellInformationProtocol::exec(SingleSeller *seller, SingleBuyer *buyer) {
	shared_signature::SharedSignature shared_signature_protocol;

	/* Generate shared secret key for ECDSA */
	shared_signature_protocol.init(&seller->shared_signature_s, &buyer->shared_signature_b);

	buyer->createT1();

	buyer->createT2();

	seller->setT2(buyer->getT2());	// TODO verify T2

	auto T2_bytes = buyer->getT2().get_bytes();

	/* Sign T2 */
	buyer->shared_signature_b.set_data(T2_bytes.data(), T2_bytes.size());

	shared_signature_protocol.exec(&seller->shared_signature_s, &buyer->shared_signature_b);

	timed_commitment::commit(&seller->timed_commitment_commiter, &buyer->timed_commitment_receiver, K, BitUtils::integer_to_bits(seller->shared_signature_s.get_ds()));

	seller->genKeysAndSetupCommits();

	sha_commitment::ShaCommitment sha_commitment_protocol;
	
	for(unsigned i = 0; i < L; ++i) {
		sha_commitment_protocol.init(&seller->keys_commits[i], &buyer->keys_commits[i]);
		sha_commitment_protocol.exec(&seller->keys_commits[i], &buyer->keys_commits[i]);
	}

	sha_commitment_protocol.init(&seller->signature_commit, &buyer->signature_commit);
	sha_commitment_protocol.exec(&seller->signature_commit, &buyer->signature_commit);
}

void SingleSellInformationProtocol::open(SingleSeller *single_seller, SingleBuyer *single_buyer) {
	/* verify 
	 * - init
	 * - signing
	 * TODO
	 */

	shared_signature::SharedSignature shared_signature_protocol;
	shared_signature_protocol.open(&single_seller->shared_signature_s, &single_buyer->shared_signature_b);

	timed_commitment::open_commitment(&single_seller->timed_commitment_commiter, &single_buyer->timed_commitment_receiver);

	sha_commitment::ShaCommitment sha_commitment_protocol;

	for(unsigned i = 0; i < L; ++i) {
		sha_commitment_protocol.open(&single_seller->keys_commits[i], &single_buyer->keys_commits[i]);
	}

	sha_commitment_protocol.open(&single_seller->signature_commit, &single_buyer->signature_commit);

	single_buyer->verifyKeys();

	single_buyer->setOpenVerified(single_buyer->shared_signature_b.getOpenVerified());
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
	
	SingleSellInformationProtocol single_sell_information;

	shared_signature::S shared_signature_s(seller->getRng(), seller->getEcParameters());
	shared_signature::B shared_signature_b(
		buyer->getRng(), 
		buyer->getEcParameters(),
		shared_signature_s.get_paillier_n(),
		shared_signature_s.get_paillier_g());

	SingleSeller single_seller(
		seller->getRng(),
		seller->getEcParameters(),
		shared_signature_s);
	SingleBuyer single_buyer(
		buyer->getRng(),
		buyer->getEcParameters(),
		shared_signature_b);

	vector<SingleSeller> single_sellers(T, single_seller);
	vector<SingleBuyer> single_buyers(T, single_buyer);

	cut_and_choose::Prover<SingleSeller> prover;
	prover.v = single_sellers;
	cut_and_choose::Verifier<SingleBuyer> verifier;
	verifier.v = single_buyers;
	verifier.i = 2;	// TODO

	cut_and_choose::cut_and_choose< SingleSellInformationProtocol, 
		SingleSeller,
		SingleBuyer,
		T > (&prover, &verifier);

	if (!verifier.res) {
		throw ProtocolException("Generating signatures and keys failed!");
	}
}

void SellInformationProtocol::open(Seller *seller, Buyer *buyer){

}

}
