CXX=clang++
CPPFLAGS=-O2 -Wall -Wextra
CXXFLAGS=-std=c++11 -stdlib=libc++
LDFLAGS=-Wl,-z,now,-z,relro
LDLIBS=-lc++abi -lboost_system -lpthread -lcrypto -lssl

SOURCES= \
	./net/async_client.cc \
	./net/client.cc \
	./oauth/account.cc \
	./oauth/oauth.cc \
	./utility/base64.cc \
	./utility/random_str.cc \
	./utility/sha1.cc \
	./utility/url.cc \
	./test.cc

OBJECTS = $(subst .cc,.o,$(SOURCES))

test: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

all: test

.PHONY: clean
clean:
	-rm test
	-rm $(OBJECTS)
