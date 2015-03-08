#ifndef TWITPP_UTILITY_H
#define TWITPP_UTILITY_H

#include <string>
#include <tuple>
#include <utility>
#include <boost/optional.hpp>

namespace twitpp {
namespace util {

std::string base64_encode(const unsigned char* data, std::size_t length);

std::string hmac_sha1_encode(const std::string& key, const std::string& data);

std::string random_str(unsigned int length);

std::string url_encode(const std::string& text);

std::string to_lower(const std::string& str);

using header_t = std::pair<std::string, std::string>;
boost::optional<header_t> header_parser(const std::string& header);

using url_t = std::tuple<std::string, std::string, std::string, std::string>;
boost::optional<url_t> url_parser(const std::string& url);

} // namespace util
} // namespace twitpp

#endif
