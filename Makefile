CXX=clang++
CFLAGS=-O2 -std=c++11 -stdlib=libc++ -Wall -Wextra
LDFLAGS=-lc++abi -lboost_system -lboost_thread -lcrypto -lssl -pthread
 
all: a.out
 
a.out : test.o asioWrapper.o oauth.o
	$(CXX) $(CFLAGS) ${LDFLAGS} test.o asioWrapper.o oauth.o

test.o : test.cc
	$(CXX) $(CFLAGS) -c test.cc

oauth.o : ./oauth/oauth.cc
	$(CXX) $(CFLAGS) -c ./oauth/oauth.cc

asioWrapper.o : ./asioWrapper/asioWrapper.cc
	$(CXX) $(CFLAGS) -c ./asioWrapper/asioWrapper.cc
 
clean:
	rm a.out *.o
