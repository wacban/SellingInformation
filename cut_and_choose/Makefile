
FLAGS=-std=c++11 -Wall -Wextra -Wno-unused

ALL=test_cc_sha_commit

all: $(ALL)

$(ALL): %: %.cpp ../sha_commitment/sha_commitment.o 
	g++ $(FLAGS) -o $@ $^ -lcryptopp

%.o: %.cpp
	g++ -c $(FLAGS) -o $@ $^ -lcryptopp
	
clean:
	rm -rf $(ALL) *.o
