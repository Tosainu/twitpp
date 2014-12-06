#include <random>
#include <string>
#include "util.h"

namespace twitpp {
namespace util {

std::string random_str(unsigned int length) {
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
