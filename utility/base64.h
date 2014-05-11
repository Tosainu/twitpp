/* 
   I refer to the following URL.
    http://d.hatena.ne.jp/blono/20100908/1283945820
*/

#ifndef TWITPP_UTILITY_BASE64_H
#define TWITPP_UTILITY_BASE64_H

#include <string>

namespace twitpp {
namespace utility {

  std::string base64Encode(const std::string& text) {
    const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;

    for(std::size_t cnt = 0; cnt < text.size(); ++cnt) {
      switch(cnt % 3) {
        case 0:
          result.push_back(base64Chars[(text[cnt] & 0xFC) >> 2]);

          if((cnt + 1) == text.size()) {
            result.push_back(base64Chars[(text[cnt] & 0x03) << 4]);
            result.push_back('=');
            result.push_back('=');
          }
          break;

        case 1:
          result.push_back(base64Chars[((text[cnt -1] & 0x03) << 4) | ((text[cnt] & 0xF0) >> 4)]);

          if((cnt + 1) == text.size()) {
            result.push_back(base64Chars[(text[cnt] & 0x0F) << 2]);
            result.push_back('=');
          }
          break;

        case 2:
          result.push_back(base64Chars[((text[cnt -1] & 0x0F) << 2) | ((text[cnt] & 0xC0) >> 6)]);
          result.push_back(base64Chars[text[cnt] & 0x3F]);

          break;
      }
    }

    return result;
  }

}
}

#endif
