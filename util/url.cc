#ifndef TWITPP_UTILITY_URL_CC
#define TWITPP_UTILITY_URL_CC

#include <sstream>
#include <string>
#include <tuple>
#include <boost/xpressive/xpressive.hpp>
#include "util.h"

namespace twitpp {
namespace util {

std::string url_encode(const std::string& text) {
  const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_.~";
  std::ostringstream result;

  for (std::string::const_iterator it = text.begin(); it != text.end(); ++it) {
    if ((charset.find(*it)) == std::string::npos) {
      std::ostringstream os;
      os.setf(std::ios::hex, std::ios::basefield);
      os.setf(std::ios::uppercase);
      os << static_cast<int>(*it);

      std::string tmp = os.str();

      if (tmp.length() > 2) tmp.erase(0, 6);

      result << "%" << tmp;
    } else {
      result << *it;
    }
  }

  return result.str();
}

std::tuple<std::string, std::string, std::string> url_parser(const std::string& url) {
  using namespace boost::xpressive;

  sregex url_parser = (s1 = +alpha) >> "://" >> (s2 = +(_w | '.')) >> (s3 = *_);
  smatch res;
  std::string path;

  if (regex_search(url, res, url_parser)) {
    return std::forward_as_tuple(res[1], res[2], res[3].length() == 0 ? static_cast<std::string>("/") : res[3]);
  } else {
    throw std::invalid_argument("failed to parse the url");
  }
}

}
}

#endif
