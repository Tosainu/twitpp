#include <random>
#include <boost/xpressive/xpressive.hpp>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include "util.h"

namespace twitpp {
namespace util {

std::string base64_encode(const std::string& text) {
  const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string result;

  for (std::size_t cnt = 0; cnt < text.size(); ++cnt) {
    switch (cnt % 3) {
      case 0:
        result.push_back(charset[(text[cnt] & 0xFC) >> 2]);

        if ((cnt + 1) == text.size()) {
          result.push_back(charset[(text[cnt] & 0x03) << 4]);
          result.push_back('=');
          result.push_back('=');
        }
        break;

      case 1:
        result.push_back(charset[((text[cnt - 1] & 0x03) << 4) | ((text[cnt] & 0xF0) >> 4)]);

        if ((cnt + 1) == text.size()) {
          result.push_back(charset[(text[cnt] & 0x0F) << 2]);
          result.push_back('=');
        }
        break;

      case 2:
        result.push_back(charset[((text[cnt - 1] & 0x0F) << 2) | ((text[cnt] & 0xC0) >> 6)]);
        result.push_back(charset[text[cnt] & 0x3F]);

        break;
    }
  }

  return result;
}

std::string hmac_sha1_encode(const std::string& key, const std::string& data) {
  return static_cast<std::string>((char*)
    HMAC(EVP_sha1(), (unsigned char*)key.c_str(), key.length(), (unsigned char*)data.c_str(), data.length(), nullptr, nullptr)
  );
}

std::string random_str(unsigned int length) {
  const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::string result;

  std::mt19937 mt{std::random_device()()};
  std::uniform_int_distribution<unsigned int> random(0, 61);

  for (unsigned int cnt = 0; cnt < length; ++cnt) {
    result.append(1, charset[random(mt)]);
  }

  return result;
}

std::string url_encode(const std::string& text) {
  std::ostringstream result;
  result.fill('0');
  result << std::hex << std::uppercase;

  for (auto&& c : text) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      result << c;
    } else {
      result << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
    }
  }

  return result.str();
}

boost::optional<url_t> url_parser(const std::string& url) {
  using namespace boost::xpressive;

  sregex url_parser = (s1 = +alpha) >> "://" >> (s2 = +(_w | '.')) >> (s3 = *~(set = '?')) >> (s4 = *_);
  smatch res;
  std::string path;

  if (regex_search(url, res, url_parser)) {
    return std::make_tuple(res.str(1), res.str(2), res[3].length() == 0 ? "/" : res.str(3), res.str(4));
  } else {
    return boost::none;
  }
}

}
}
