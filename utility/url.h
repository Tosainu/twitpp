#ifndef TWITPP_UTILITY_URL_H
#define TWITPP_UTILITY_URL_H

#include <string>

namespace twitpp {
namespace utility {

class url {
public:
  std::string encode(const std::string& text);
};

} // utility
} // twitpp

#endif
