
FLAGS=-std=c++11 -Wall -Wextra -Wno-unused

ALL=test_timed_commitment test_regular_commit
#test_safe_prime

all: $(ALL)

$(ALL): %: %.cpp regular_commitment.o timed_commitment.o 
	g++ $(FLAGS) -o $@ $^ -lcryptopp

%.o: %.cpp
	g++ -c $(FLAGS) -o $@ $^ -lcryptopp
	
clean:
	rm -rf $(ALL) *.o
