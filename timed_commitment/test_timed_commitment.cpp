
#include "timed_commitment.h"

#include <iostream>
#include <ctime>

using namespace CryptoPP;
using namespace std;
using namespace TimedCommitment;

vector<bool> to_bits(const string &s){
  vector<bool> res;
  for(char c: s){
    for(unsigned i = 0; i < 8; ++i){
      res.push_back(c & (1<<i));
    }
  }
  return res;
}

string from_bits(const vector<bool> bits){
  unsigned l = bits.size();
  if (l % 8 != 0){
    cerr << "from bits" << endl;
  }
  string res;
  for(unsigned i = 0; i < l; i += 8){
    unsigned char x = 0;
    for(unsigned bit = 0; bit < 8; ++bit){
      if (bits[i+bit])
        x += 1 << bit;
    }
    res += x;
  }
  return res;
}

void print_bits(vector<bool> bits){
  cout << "bits:   ";
  for(bool b: bits){
    cout << (unsigned) b;
  }
  cout << endl;
}

void print_string(string s){
  cout << "string: ";
  for(char c: s){
    cout << (unsigned)c << "\t";
  }
  cout << endl;
}

void test_bits(){

  string m = "a";
  auto bits = to_bits(m);
  // print_bits(bits);
  
  string m2 = from_bits(bits);
  // print_string(m);
  // print_string(m2);

  string s = "adasadsz";

  if (s != from_bits(to_bits(s))){
    cerr << "ERR test bits" << endl;
  } else {
    // cerr << "OK test bits" << endl;
  }
}

void test(AutoSeededRandomPool &rng, int K, string m, bool t = false){

  time_t start, end; 

  if (t){
    cerr << "--- " << "K : " << K << " ------------" << endl;
  }

  start = time(NULL);
  Commiter c(&rng);
  end = time(NULL);

  if (t){
    cerr << "init   " << difftime(end, start) << endl;
  }

  // cerr << "commit" << endl;
  start = time(NULL);
  Commitment com = c.commit(K, to_bits(m));

  // cerr << "receiver" << endl;
  Receiver r(&rng);
	r.accept_commitment(com);
  
  // cerr << "zk" << endl;
  for(int i = 0; i < 10; ++i){
    vector<RegularCommitment> commits = r.zk_1();
    vIvI zw = c.zk_2(commits);
    vI commit_values = r.zk_3(zw);
    vI y = c.zk_4(commit_values);
    r.zk_5(y);
  }
  end = time(NULL);

  if (t){
    cerr << "commit " << difftime(end, start) << endl;
  }

  // cerr << "open" << endl;
  Integer vp = c.open();

  // cerr << "open" << endl;
  string m2 = from_bits(r.open(vp));

  if (m != m2){
    cerr << "FAIL open" << endl;
  } else {
    // cerr << "OK" << endl;
  }
  
  start = time(NULL);
  // cerr << "force open" << endl;
  string m3 = from_bits(r.force_open());
  if (m != m3){
    cerr << "FAIL force open" << endl;
  } else {
    // cerr << "OK" << endl;
  }
  end = time(NULL);

  if (t){
    cerr << "force open " << difftime(end, start) << endl;
  }

  start = time(NULL);
  // cerr << "force open smart" << endl;
  string m4 = from_bits(r.force_open_smart());
  if (m != m4){
    cerr << "FAIL force open smart" << endl;
  } else {
    // cerr << "OK" << endl;
  }

  end = time(NULL);

  if (t){
    cerr << "force open smart" << difftime(end, start) << endl;
  }

  if (t){
    cerr << "--------------------" << endl;
  }
}

int test2(AutoSeededRandomPool &rng, int K, string m){

  Commiter c(&rng);
  Receiver r(&rng);
	commit(&c, &r, 16, to_bits(m));

	string m1 = from_bits(open_commitment(&c, &r));
	if (m1 != m){
		cerr << "ERR open failed" << endl;
		return 1;
	}

	string m2 = from_bits(r.force_open());
	if (m2 != m){
		cerr << "ERR force open failed" << endl;
		return 1;
	}

	string m3 = from_bits(r.force_open_smart());
	if (m3 != m){
		cerr << "ERR force open smart failed" << endl;
		return 1;
	}

	return 0;
}

int main(){
  AutoSeededRandomPool rng;
  string m = "hello world";

  test_bits();

  cerr << "small test" << endl;
  for(int i = 0; i < 10; ++i){
    cerr << i + 1 << "/" << 10 << endl;
    byte m[i+5+1];
    m[i+5] = '\0';
    rng.GenerateBlock(m, i+5);
    test(rng, 18, string((char *)m));
  }

	cerr << "small test 2" << endl;
	for(int i = 0; i < 10; ++i){
	  cerr << i + 1 << "/" << 10 << endl;
    byte m[i+5+1];
    m[i+5] = '\0';
    rng.GenerateBlock(m, i+5);
    test2(rng, 18, string((char *)m));
	}

  cerr << "time test" << endl;
  for(int K = 18; K < 21; ++K){
    string m = "hello world";
    test(rng, K, m, true);
  }
}

