# twitpp [![Build Status](https://travis-ci.org/Tosainu/twitpp.svg?branch=master)](https://travis-ci.org/Tosainu/twitpp)

A Twitter Library for C++11.

## Requirement

* GCC 4.8.0+ or Clang 3.4+
* Boost C++ Libraries 1.55.0+
* OpenSSL
* CMake 2.8.0+

## Install

```
$ mkdir -p build && cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ make
$ sudo make install
```

## Usage

### OAuth Authentication

```cpp
twitpp::oauth::account account("CONSUMER", "CONSUMER_SECRET");

// get authorize_url
account.get_authorize_url();
std::cout << account.authorize_url() << std::endl;

// get oauth token
account.get_oauth_token("PIN")

std::cout << "consumer_key:    " << account.consumer_key()       << std::endl;
std::cout << "consumer_secret: " << account.consumer_secret()    << std::endl;
std::cout << "oauth_token:     " << account.oauth_token()        << std::endl;
std::cout << "oauth_secret:    " << account.oauth_token_secret() << std::endl;
```

### Post Status

```cpp
twitpp::oauth::client oauth(twitpp::oauth::account("CK", "CS", "AT", "AS"));

auto res = oauth.post("https://api.twitter.com/1.1/statuses/update.json",
                      {{"status", "Hello World!!"}});
std::cout << res.body << std::endl;
```

### Streaming API

```cpp
twitpp::oauth::client oauth(twitpp::oauth::account("CK", "CS", "AT", "AS"));

oauth.stream_get("https://userstream.twitter.com/1.1/user.json",
                 [](twitpp::net::response& response) {
  std::cout << response.body << std::endl;
  response.body.clear();
});
```

also, see [example.cc](example/example.cc).

## License

twitpp is licensed under the [MIT license](LICENSE).
