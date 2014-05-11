#ifndef TWITPP_UTILITY_NUMTOSTR_H
#define TWITPP_UTILITY_NUMTOSTR_H

#include <string>
#include <sstream>

namespace twitpp {
namespace utility {

  template<typename Num>
  std::string numToStr(const Num& num) {
    std::ostringstream tmp;
    tmp << num;
    return tmp.str();
  }

}
}

#endif
