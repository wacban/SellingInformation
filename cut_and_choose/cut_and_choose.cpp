
#include "cut_and_choose.h"

template <typename P, typename A, typename B, unsigned T>
void cut_and_choose(Prover<A> *p, Verifier<B> *v){
	P protocol;
	for(int i = 0; i < T; ++i){
		protocol.init(&p->v[i], &v->v[i]);
		protocol.exec(&p->v[i], &v->v[i]);
	}

	int i = v->getI();
	v->res = true;
	for(int i = 0; i < T; ++i){
		protocol.open(&p->v[i], &v->v[i]);
		if (!v->v[i].openVerified()){
			v->res = false;
			return;
		}
	}
}


