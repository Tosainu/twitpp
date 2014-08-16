#ifndef TWITPP_UTILITY_SHA1_CC
#define TWITPP_UTILITY_SHA1_CC

#include <string>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include "sha1.h"

namespace twitpp {
namespace utility {

std::string hmac_sha1::encode(const std::string& key, const std::string& data) {
  unsigned char* res;

  res = HMAC(EVP_sha1(), (const unsigned char*)key.c_str(), key.length(),
      (const unsigned char*)data.c_str(), data.length(), NULL, NULL);

  std::string result((char*)res);
  return result;
}

}
}

#endif
