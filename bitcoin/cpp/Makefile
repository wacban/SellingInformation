GEN_SRC := gen-cpp/BitcoinUtils.cpp gen-cpp/bitcoin_utils_constants.cpp gen-cpp/bitcoin_utils_types.cpp
GEN_OBJ := $(patsubst %.cpp,%.o, $(GEN_SRC))

THRIFT_DIR := /usr/local/include/thrift
BOOST_DIR := /usr/local/include

INC := -I$(THRIFT_DIR) -I$(BOOST_DIR)

.PHONY: all generate clean 

all: generate bitcoin_utils_client

generate:
	thrift --gen cpp ../bitcoin_utils.thrift

%.o: %.cpp
	$(CXX) -Wall -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H $(INC) -c $< -o $@

bitcoin_utils_client: bitcoin_utils_client.o $(GEN_OBJ)
	$(CXX) $^ -o $@ -L/usr/local/lib -lthrift 

clean:
	rm -rf *.o gen-cpp/*.o bitcoin_utils_client gen-cpp
