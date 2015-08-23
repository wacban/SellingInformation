#ifndef _SHARED_SIG_H_
#define _SHARED_SIG_H_

#include <cryptopp/ecp.h>
#include <cryptopp/oids.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/osrng.h>

#include "paillier.h"

namespace SharedSignature {

	class ProtocolException: public std::exception {
		std::string msg;
		public:
		ProtocolException(const std::string &msg):msg(msg){
		}
		virtual const char* what() const throw(){
			return msg.c_str();
		}
	};

	// hash message
	// take ret_byte_count leftmost bytes
	// convert to Integer
	CryptoPP::Integer m_to_int(byte *m, unsigned m_len, unsigned ret_byte_count);

	class S {
		private:

			CryptoPP::AutoSeededRandomPool *rng;

			CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> ec_parameters;
			CryptoPP::ECP ec;
			CryptoPP::ECPPoint G; // subgroup generator - base point
			CryptoPP::Integer n;  // subgroup order

			CryptoPP::Integer ds;
			CryptoPP::Integer ks;

			CryptoPP::ECPPoint Qs;
			CryptoPP::ECPPoint Qb;

			CryptoPP::ECPPoint Q;

			CryptoPP::ECPPoint Ks;

			Paillier paillier;

			CryptoPP::Integer cs;

			CryptoPP::Integer r;
			CryptoPP::Integer s;

		public:

			S(){}

			S(CryptoPP::AutoSeededRandomPool *rng, CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> ec_parameters);

			CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> get_ec_parameters(){
				return ec_parameters;
			}

			CryptoPP::Integer reveal_ds(){
				return ds;
			}

			CryptoPP::ECPPoint get_Qs(){
				return Qs;
			}

			CryptoPP::ECPPoint get_Qb(){
				return Qb;
			}

			CryptoPP::ECPPoint get_Q(){
				return Q;
			}

			CryptoPP::Integer get_paillier_n(){
				return paillier.get_n();
			}

			CryptoPP::Integer get_paillier_g(){
				return paillier.get_g();
			}

			CryptoPP::ECPPoint get_Ks(){
				return Ks;
			}

			CryptoPP::Integer get_cs(){
				return cs;
			}

			CryptoPP::Integer get_r(){
				return r;
			}

			CryptoPP::Integer get_s(){
				return s;
			}
			
			std::vector<byte> get_signature(){
				std::vector<byte> signature;
				signature.resize(64);
				r.Encode(signature.data(), 32);
				s.Encode(signature.data()+32, 32);

				return signature;
			}

			void start_init();

			void finish_init(CryptoPP::ECPPoint Qb);

			void start_sig();

			void finish_sig(CryptoPP::Integer r, CryptoPP::Integer cb);
	};

	class B {
		private:

			CryptoPP::AutoSeededRandomPool *rng;

			CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> ec_parameters;
			CryptoPP::ECP ec;
			CryptoPP::ECPPoint G; // subgroup generator - base point
			CryptoPP::Integer n;  // subgroup order

			CryptoPP::Integer db;
			CryptoPP::Integer kb;

			CryptoPP::ECPPoint Qs;
			CryptoPP::ECPPoint Qb;

			CryptoPP::ECPPoint Q;

			Paillier paillier;

			CryptoPP::ECPPoint K;
			CryptoPP::Integer r;
			CryptoPP::Integer cb;

		public:
			B(){
			}

			B(CryptoPP::AutoSeededRandomPool *rng,
				CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> ec_parameters,
			 	CryptoPP::Integer paillier_n,
				CryptoPP::Integer paillier_g);

			CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> get_ec_parameters(){
				return ec_parameters;
			}

			CryptoPP::ECPPoint get_Qs(){
				return Qs;
			}

			CryptoPP::ECPPoint get_Qb(){
				return Qb;
			}

			CryptoPP::ECPPoint get_Q(){
				return Q;
			}

			CryptoPP::Integer get_r(){
				return r;
			} 

			CryptoPP::Integer get_cb(){
				return cb;
			}

			void start_init();

			void finish_init(CryptoPP::ECPPoint Qs);

			void cont_sig(CryptoPP::ECPPoint Ks, CryptoPP::Integer cs, byte *m, unsigned m_len);
	};

	void init(S *s, B *b);
	void sign(S *s, B *b, byte *data, unsigned data_length);
}

#endif
