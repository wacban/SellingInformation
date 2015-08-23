
#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

// Define protocol and classes A and B.
// For the protocol to be used in cut and choose
// B needs to define bool field valid.

template<typename A, typename B>
class Protocol {
	public:
	virtual void init(A *, B *) = 0;
	virtual void exec(A *, B *) = 0;
	virtual void open(A *, B *) = 0;
};

#endif
