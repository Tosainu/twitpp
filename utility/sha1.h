#ifndef TWITPP_UTILITY_SHA1_H
#define TWITPP_UTILITY_SHA1_H

#include <string>
#include <openssl/evp.h>
#include <openssl/hmac.h>

namespace twitpp {
namespace utility {

  std::string hmacSha1(const std::string& key, const std::string& data) {
    unsigned char* res;

    res = HMAC (EVP_sha1(), (const unsigned char*)key.c_str(), key.length(), (const unsigned char*)data.c_str(), data.length() , NULL, NULL);

    std::string result((char*)res);
    return result;
  }

}
}

#endif
