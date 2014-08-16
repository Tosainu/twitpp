#ifndef TWITPP_UTILITY_SHA1_H
#define TWITPP_UTILITY_SHA1_H

#include <string>
#include <openssl/evp.h>
#include <openssl/hmac.h>

namespace twitpp {
namespace utility {

class hmac_sha1 {
public:
  std::string encode(const std::string& key, const std::string& data);
};

} // utility
} // twitpp

#endif
