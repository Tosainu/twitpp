#ifndef TWITPP_UTILITY_SHA1_CC
#define TWITPP_UTILITY_SHA1_CC

#include <string>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include "util.h"

namespace twitpp {
namespace util {

std::string hmac_sha1_encode(const std::string& key, const std::string& data) {
  return static_cast<std::string>((char*)
    HMAC(EVP_sha1(), (unsigned char*)key.c_str(), key.length(), (unsigned char*)data.c_str(), data.length(), nullptr, nullptr)
  );
}

}
}

#endif
