#ifndef TWITPP_UTILITY_H
#define TWITPP_UTILITY_H

#include <string>
#include <tuple>

namespace twitpp {
namespace util {

std::string random_str(unsigned int length);

std::string base64_encode(const std::string& text);

std::string hmac_sha1_encode(const std::string& key, const std::string& data);

std::string url_encode(const std::string& text);

std::tuple<bool, std::string, std::string, std::string, std::string> url_parser(const std::string& url);

}
}

#endif
