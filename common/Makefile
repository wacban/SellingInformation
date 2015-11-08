
FLAGS=-std=c++11 -Wall -Wextra -Wno-unused

ALL=common.o

all: $(ALL)

%.o: %.cpp
	g++ -c $(FLAGS) -o $@ $^ -lcryptopp
	
clean:
	rm -rf $(ALL) *.o
