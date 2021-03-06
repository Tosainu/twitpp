#include <algorithm>
#include <random>
#include <boost/xpressive/xpressive.hpp>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include "util.h"
#include "query_parser.h"

namespace twitpp {
namespace util {

const static char charset[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                               'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                               'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                               'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

std::string base64_encode(const unsigned char* data, const unsigned int& length) {
  std::string result;

  for (auto i = 0u; i < length; ++i) {
    switch (i % 3) {
      case 0:
        result.push_back(charset[(data[i] & 0xFC) >> 2]);

        if ((i + 1) == length) {
          result.push_back(charset[(data[i] & 0x03) << 4]);
          result.push_back('=');
          result.push_back('=');
        }
        break;

      case 1:
        result.push_back(charset[((data[i - 1] & 0x03) << 4) | ((data[i] & 0xF0) >> 4)]);

        if ((i + 1) == length) {
          result.push_back(charset[(data[i] & 0x0F) << 2]);
          result.push_back('=');
        }
        break;

      case 2:
        result.push_back(charset[((data[i - 1] & 0x0F) << 2) | ((data[i] & 0xC0) >> 6)]);
        result.push_back(charset[data[i] & 0x3F]);

        break;
    }
  }

  return result;
}

std::string to_lower(const std::string& str) {
  std::string result(str.length(), 0);
  std::transform(str.begin(), str.end(), result.begin(), ::tolower);

  return result;
}

std::string hmac_sha1_encode(const std::string& key, const std::string& data) {
  unsigned char result[SHA_DIGEST_LENGTH + 1];
  unsigned int length;

  HMAC(EVP_sha1(),
       reinterpret_cast<const unsigned char*>(key.c_str()), key.length(),
       reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
       result, &length);

  return base64_encode(result, length);
}

std::string random_str(const std::size_t& length) {
  std::string result(length, 0);

  std::mt19937 mt{std::random_device()()};
  std::uniform_int_distribution<unsigned int> random(0, 61);

  std::generate_n(result.begin(), length, [&] {
    return charset[random(mt)];
  });

  return result;
}

std::string url_encode(const std::string& str) {
  std::ostringstream result;
  result.fill('0');
  result << std::hex << std::uppercase;

  for (const auto& c : str) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      result << c;
    } else {
      result << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
    }
  }

  return result.str();
}

boost::optional<header_t> header_parser(const std::string& header) {
  using namespace boost::xpressive;

  const static sregex regex = (s1 = +graph) >> *space >> ':' >> *space >> (s2 = +print) >> *cntrl;
  smatch res;

  if (regex_search(header, res, regex)) {
    return std::make_pair(to_lower(res.str(1)), res.str(2));
  } else {
    return boost::none;
  }
}

boost::optional<url_t> url_parser(const std::string& url) {
  using namespace boost::xpressive;

  const static sregex regex = (s1 = +alpha) >> "://" >> (s2 = +(_w | '.')) >> (s3 = *~(set = '?')) >> (s4 = *_);
  smatch res;

  if (regex_search(url, res, regex)) {
    return std::make_tuple(res.str(1), res.str(2), res[3].length() == 0 ? "/" : res.str(3), res.str(4));
  } else {
    return boost::none;
  }
}

std::unordered_map<std::string, std::string> parse_query_str(const std::string& str) {
  std::unordered_map<std::string, std::string> res{};
  qi::parse(str.begin(), str.end(), query_parser<std::string>(), res);
  return res;
}

} // namespace util
} // namespace twitpp
