#ifndef TWITPP_UTILITY_RANDOMSTR_H
#define TWITPP_UTILITY_RANDOMSTR_H

#include <random>
#include <string>

namespace twitpp {
namespace utility {

class RandomStr {
public:
  std::string operator()(unsigned int length);
};

} // utility
} // twitpp

#endif
