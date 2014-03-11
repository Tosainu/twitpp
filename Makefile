a.out : libTest.o twitpp.o
	g++ libTest.o twitpp.o -Wall -Wextra -lcrypto -lcurl -std=gnu++11
libTest.o : libTest.cc
	g++ -c libTest.cc -Wall -Wextra -lcrypto -lcurl -std=gnu++11
twitpp.o : twitpp.cc
	g++ -c twitpp.cc -Wall -Wextra -lcrypto -lcurl -std=gnu++11

