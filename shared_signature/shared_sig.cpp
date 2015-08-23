#include <cryptopp/ecp.h>
#include <cryptopp/oids.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>

#include <iostream>

#include "shared_sig.h"

#include "paillier.h"

using namespace CryptoPP;

using namespace std;

namespace SharedSignature{

	Integer m_to_int(byte *m, unsigned m_len, unsigned ret_byte_count){
		SHA256 sha256;
		byte h[sha256.DigestSize()];
		SHA256().CalculateDigest(h, m, m_len);
		return Integer (h, ret_byte_count);
	}

	/**************************
	 * S
	 **************************/
	S::S(AutoSeededRandomPool *rng, DL_GroupParameters_EC<ECP> ec_parameters):
		rng(rng),
		ec_parameters(ec_parameters),
		ec(ec_parameters.GetCurve()),
		G(ec_parameters.GetSubgroupGenerator()),
		n(ec_parameters.GetGroupOrder()), 
		paillier(rng, 2*n.BitCount() + 3) {
		// check that paillier has good bit length
		if (paillier.get_n() <= 2*n*n*n*n){
			cerr << "too short paillier" << endl;
			cerr << "S ctor" << endl
				<< "n      " << paillier.get_n().BitCount() << endl
				<< "2*p**4 " << (2*n*n*n*n).BitCount() << endl;
		}
		// TODO throw
	}

	void S::start_init(){
		ds.Randomize(*rng, Integer::One(), n-1);
		Qs = ec.Multiply(ds, G);
	}

	void S::finish_init(ECPPoint Qb){
		this->Qb = Qb;
		this->Q = ec.Multiply(ds, Qb);
	}

	void S::start_sig(){
		ks.Randomize(*rng, Integer::One(), n-1);
		Ks = ec.Multiply(ks, G);
		cs = paillier.enc(ds);
	}

	void S::finish_sig(Integer r, Integer cb){
		this->r = r;
		Integer s2 = paillier.dec(cb) % n;
		s = a_times_b_mod_c(ks.InverseMod(n), s2, n);
		if (s == 0){
			cerr << "ERR s==0 restart protocol" << endl;
		}
	}

	/**************************
	 * B
	 **************************/

	B::B(	AutoSeededRandomPool *rng,
				DL_GroupParameters_EC<ECP> ec_parameters, 
				Integer paillier_n, 
				Integer paillier_g): 
		rng(rng),
		ec_parameters(ec_parameters),
		ec(ec_parameters.GetCurve()),
		G(ec_parameters.GetSubgroupGenerator()),
		n(ec_parameters.GetGroupOrder()),
		paillier(rng, paillier_n, paillier_g) {

			// TODO throw or exit
			if (paillier.get_n() <= 2*n*n*n*n)
				cerr << "too short paillier" << endl;
		}

	void B::start_init(){
		db.Randomize(*rng, Integer::One(), n-1);
		Qb = ec.Multiply(db, G);
	}

	void B::finish_init(ECPPoint Qs){
		this->Qs = Qs;
		this->Q = ec.Multiply(db, Qs);
	}

	void B::cont_sig(ECPPoint Ks, Integer cs, byte *m, unsigned m_len){
		kb.Randomize(*rng, Integer::One(), n-1);
		K = ec.Multiply(kb, Ks);
		r = K.x % n;
		if (r == 0)
			cerr << "ERR r==0" << endl;; // TODO

		Integer hi = m_to_int(m, m_len, n.ByteCount());
		Integer c1 = paillier.enc( a_times_b_mod_c(kb.InverseMod(n), hi, n) );
		Integer t = a_times_b_mod_c(a_times_b_mod_c(kb.InverseMod(n), r, n), db, n);
		Integer c2 = a_times_b_mod_c(c1, a_exp_b_mod_c(cs, t, paillier.get_n2()), paillier.get_n2());
		Integer u(*rng, Integer::Zero(), n*n - 1);
		cb = a_times_b_mod_c(c2, paillier.enc(u*n), paillier.get_n2());
	}

	void init(S *s, B *b){
		s->start_init();
		b->start_init();

		s->finish_init(b->get_Qb());
		b->finish_init(s->get_Qs());

		if (!(s->get_Q() == b->get_Q())){
			throw ProtocolException("Mismatching Q");
		}
	}

	void sign(S *s, B *b, byte *data, unsigned data_length){
		s->start_sig();

		b->cont_sig(s->get_Ks(), s->get_cs(), data, data_length);

		s->finish_sig(b->get_r(), b->get_cb());

		byte signature[64];
		s->get_r().Encode(signature, 32);
		s->get_s().Encode(signature+32, 32);

		ECDSA<ECP, SHA256>::PublicKey publicKey;
		publicKey.Initialize(ASN1::secp256k1(), s->get_Q());

		ECDSA<ECP, SHA256>::Signer signer;
		ECDSA<ECP, SHA256>::Verifier verifier(publicKey);

		bool result = verifier.VerifyMessage(data, data_length, signature, 64);
		if (!result){
			throw ProtocolException("Invalid signature generated!");
		}
	}
}
