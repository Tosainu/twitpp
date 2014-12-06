#include <sstream>
#include <string>
#include <tuple>
#include <boost/xpressive/xpressive.hpp>
#include "util.h"

namespace twitpp {
namespace util {

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

std::tuple<bool, std::string, std::string, std::string> url_parser(const std::string& url) {
  using namespace boost::xpressive;

  sregex url_parser = (s1 = +alpha) >> "://" >> (s2 = +(_w | '.')) >> (s3 = *_);
  smatch res;
  std::string path;

  if (regex_search(url, res, url_parser)) {
    return std::forward_as_tuple(true, res[1], res[2], res[3].length() == 0 ? static_cast<std::string>("/") : res[3]);
  } else {
    return std::make_tuple(false, std::string(""), std::string(""), std::string(""));
  }
}

}
}
