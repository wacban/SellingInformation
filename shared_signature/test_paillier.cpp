#include <iostream>
#include "cryptopp/integer.h"
// #include "cryptopp/osrng.h"

#include "paillier.h"

// ok 
const int TEST_COUNT = 1000;
const int BITS = 200;

using CryptoPP::Integer;
using namespace std;

static void test_paillier(CryptoPP::AutoSeededRandomPool *rng, Paillier &paillier, Paillier &client){

  for(int i = 0; i < TEST_COUNT; ++i){
    if (i % (TEST_COUNT/10) == 0 ) cerr << i/10 << "%" << endl;
    Integer m(*rng, Integer::Zero(), paillier.get_n());
    Integer c = paillier.enc(m);
    Integer cc = client.enc(m);
    Integer m2 = paillier.dec(c);
    Integer m2c = paillier.dec(cc);
    if (m2 != m || m2c != m){
      cerr << "pailler err on: " << m << endl;
      return ;
    }
  }
  cout << "OK" << endl;
}

static void test_homo_paillier(CryptoPP::AutoSeededRandomPool *rng, Paillier &paillier, Paillier &client){
  Integer n = paillier.get_n();
  Integer n2 = n * n;
  for(int i = 0; i < TEST_COUNT; ++i){
    if (i % (TEST_COUNT/10) == 0) cerr << i/10 << "%" << endl;
    Integer m1(*rng, Integer::Zero(), paillier.get_n());
    Integer m2(*rng, Integer::Zero(), paillier.get_n());
    Integer c1 = paillier.enc(m1);
    Integer c2 = paillier.enc(m2);
    Integer c1c = client.enc(m1);
    Integer c2c = client.enc(m2);

    Integer c12 = a_times_b_mod_c(c1, c2, n2);
    Integer c12c = a_times_b_mod_c(c1c, c2c, n2);

    Integer m12 = paillier.dec(c12);
    Integer m12c = paillier.dec(c12c);
    if ( (m12 != (m1 + m2) % n) || (m12c != (m1+m2) % n) ){
      cerr << "homo paillier err on: " << m1 << " " << m2 << endl;
      return;
    }
  }
  cout << "OK" << endl;
}

int main( int, char** ) {

  CryptoPP::AutoSeededRandomPool rng;
  Paillier paillier(&rng, BITS);
  Paillier client(&rng, paillier.get_n(), paillier.get_g());

  cout << "test 1/2" << endl;
  test_paillier(&rng, paillier, client);
  cout << "test 2/2" << endl;
  test_homo_paillier(&rng, paillier, client);

  return 0;
}

