/*
   I refer to the following URL.
    http://d.hatena.ne.jp/blono/20100908/1283945820
*/

#ifndef TWITPP_UTILITY_BASE64_H
#define TWITPP_UTILITY_BASE64_H

#include <string>

namespace twitpp {
namespace utility {

class base64 {
public:
  std::string encode(const std::string& text);
};

}  // utility
}  // twitpp

#endif
