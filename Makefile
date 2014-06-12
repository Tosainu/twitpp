CXX=clang++
CFLAGS=-O2 -std=c++11 -stdlib=libc++ -Wall -Wextra
LDFLAGS=-Wl,-z,now,-z,relro -lc++abi -lboost_system -lpthread -lcrypto -lssl

all: a.out
 
a.out : test.o asioWrapper.o oauth.o base64.o random_str.o sha1.o url.o
	$(CXX) $(CFLAGS) ${LDFLAGS} test.o asioWrapper.o oauth.o base64.o random_str.o sha1.o url.o

test.o : test.cc
	$(CXX) $(CFLAGS) -c test.cc

oauth.o : ./oauth/oauth.cc
	$(CXX) $(CFLAGS) -c ./oauth/oauth.cc

asioWrapper.o : ./asioWrapper/asioWrapper.cc
	$(CXX) $(CFLAGS) -c ./asioWrapper/asioWrapper.cc
 
base64.o : ./utility/base64.cc
	$(CXX) $(CFLAGS) -c ./utility/base64.cc
 
random_str.o : ./utility/random_str.cc
	$(CXX) $(CFLAGS) -c ./utility/random_str.cc
 
sha1.o : ./utility/sha1.cc
	$(CXX) $(CFLAGS) -c ./utility/sha1.cc
 
url.o : ./utility/url.cc
	$(CXX) $(CFLAGS) -c ./utility/url.cc
 
clean:
	rm a.out *.o
