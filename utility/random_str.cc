#ifndef TWITPP_UTILITY_RANDOMSTR_CC
#define TWITPP_UTILITY_RANDOMSTR_CC

#include <random>
#include <string>
#include "random_str.h"

namespace twitpp {
namespace utility {

std::string random_str::operator()(unsigned int length) {
  const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::string result;

  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<unsigned int> random(0, 61);

  for (unsigned int cnt = 0; cnt < length; ++cnt) {
    result.append(1, charset[random(mt)]);
  }

  return result;
}

}
}

#endif
