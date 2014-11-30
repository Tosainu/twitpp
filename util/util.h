#ifndef TWITPP_UTILITY_H
#define TWITPP_UTILITY_H

namespace twitpp {
namespace util {

std::string random_str(unsigned int length);

std::string base64_encode(const std::string& text);

std::string hmac_sha1_encode(const std::string& key, const std::string& data);

std::string url_encode(const std::string& text);

}
}

#endif
