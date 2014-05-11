#ifndef TWITPP_UTILITY_URL_H
#define TWITPP_UTILITY_URL_H

#include <string>
#include <sstream>

namespace twitpp {
namespace utility {

  std::string urlEncode(const std::string& text) {
    const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_.~";
    std::ostringstream result;

    for(std::string::const_iterator it = text.begin(); it != text.end(); ++it) {
      if((charset.find(*it)) == std::string::npos) {
        std::ostringstream os;
        os.setf(std::ios::hex, std::ios::basefield);
        os.setf(std::ios::uppercase);
        os << static_cast<int>(*it);

        std::string tmp = os.str();

        if(tmp.length() > 2)
          tmp.erase(0,6);

        result << "%" << tmp;
      }
      else {
        result << *it;
      }
    }

    return result.str();
  }

}
}

#endif
