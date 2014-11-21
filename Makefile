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
	./utility/base64.cc \
	./utility/random_str.cc \
	./utility/sha1.cc \
	./utility/url.cc \
	./test.cc

OBJECTS = $(subst .cc,.o,$(SOURCES))

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

all: $(TARGET)

.PHONY: clean
clean:
	-@rm -fv $(TARGET) $(OBJECTS)
