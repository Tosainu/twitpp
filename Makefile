CXX = g++
CXXFLAGS = -std=c++11
CPPFLAGS = -Wall -Wextra
LDFLAGS = -Wl,-z,now,-z,relro
LDLIBS = -lboost_system -lcrypto -lssl -pthread

TARGET = a.out

SOURCES = \
	./net/async_client.cc \
	./net/client.cc \
	./oauth/account.cc \
	./oauth/oauth.cc \
	./util/base64.cc \
	./util/random_str.cc \
	./util/sha1.cc \
	./util/url.cc \
	./test.cc

OBJECTS = $(subst .cc,.o,$(SOURCES))

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

all: $(TARGET)

.PHONY: clean
clean:
	-@rm -fv $(TARGET) $(OBJECTS)
