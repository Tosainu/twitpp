#ifndef TWITPP_UTILITY_RANDOMSTR_H
#define TWITPP_UTILITY_RANDOMSTR_H

#include <string>
#include <random>

namespace twitpp {
namespace utility {

  std::string randomStr(unsigned int length) {
    const std::string charset("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    std::string result;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<unsigned int> random(0, charset.length() -1);

    for(unsigned int cnt = 0; cnt < length; ++cnt) {
      result += charset[random(mt)];
    }

    return result;
  }

}
}

#endif
