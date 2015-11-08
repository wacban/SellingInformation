
FLAGS=-std=c++11 -Wall -Wextra -Wno-unused

ALL=test_square_root

all: $(ALL)

$(ALL): %: %.cpp square_root.o
	g++ $(FLAGS) -o $@ $^ -lcryptopp

%.o: %.cpp
	g++ -c $(FLAGS) -o $@ $^ -lcryptopp
	
clean:
	rm -rf $(ALL) *.o
