#include <random>
#include "util.h"

namespace twitpp {
namespace util {

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

}
}
