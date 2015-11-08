
FLAGS=-std=c++11 -Wall -Wextra -Wno-unused

ALL=common.o square_root.o aes.o test_square_root test_aes

all: $(ALL)

test_square_root: test_square_root.cpp square_root.o
	g++ $(FLAGS) -o $@ $^ -lcryptopp

test_aes: test_square_root.cpp aes.o square_root.o
	g++ $(FLAGS) -o $@ $^ -lcryptopp

%.o: %.cpp
	g++ -c $(FLAGS) -o $@ $^ -lcryptopp
	
clean:
	rm -rf $(ALL) *.o
