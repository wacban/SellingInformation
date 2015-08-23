#include "sha_commitment.h"


#include <cryptopp/integer.h>
#include <cryptopp/sha.h>

#include <iostream>

using namespace CryptoPP;
using namespace std;

namespace sha_commitment {

static void compute_sha(byte* dst, Integer x, byte* seed){
	
  unsigned len = x.MinEncodedSize();
  byte m[len + N];
  x.Encode(m, len);
	copy(seed, seed + N, m + len);
  SHA256 sha256;
  SHA256().CalculateDigest(dst, m, len+N);
}

void ShaCommitment::init(Sender *, Receiver *) {
	
}

void ShaCommitment::exec(Sender *s, Receiver *r) {

	s->rng->GenerateBlock(s->seed, N);
	
	compute_sha(s->com, s->m, s->seed);

	copy(s->com, s->com+COM_SIZE, r->com);
}

void ShaCommitment::open(Sender *s, Receiver *r) {
	
	r->m = s->m;
	copy(s->seed, s->seed + N, r->seed);
	
	byte com[COM_SIZE];
	compute_sha(com, r->m, r->seed);
	r->setOpenVerified(equal(r->com, r->com + COM_SIZE, com));
}

/*
static RegularCommitment sha256(const Integer &x){
  
  unsigned len = x.MinEncodedSize();
  RegularCommitment res;
  byte m[len];
  x.Encode(m, len);
  SHA256 sha256;
  SHA256().CalculateDigest(res.digest, m, len);
  return res;
}

RegularCommitment regular_commit(const Integer &x){
  return sha256(x);
}

bool regular_verify(const RegularCommitment &c, const Integer &x){
  return c == sha256(x);
}
*/

}
