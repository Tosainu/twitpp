#include <string>
#include "util.h"

namespace twitpp {
namespace util {

std::string base64_encode(const std::string& text) {
  const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string result;

  for (std::size_t cnt = 0; cnt < text.size(); ++cnt) {
    switch (cnt % 3) {
      case 0:
        result.push_back(charset[(text[cnt] & 0xFC) >> 2]);

        if ((cnt + 1) == text.size()) {
          result.push_back(charset[(text[cnt] & 0x03) << 4]);
          result.push_back('=');
          result.push_back('=');
        }
        break;

      case 1:
        result.push_back(charset[((text[cnt - 1] & 0x03) << 4) | ((text[cnt] & 0xF0) >> 4)]);

        if ((cnt + 1) == text.size()) {
          result.push_back(charset[(text[cnt] & 0x0F) << 2]);
          result.push_back('=');
        }
        break;

      case 2:
        result.push_back(charset[((text[cnt - 1] & 0x0F) << 2) | ((text[cnt] & 0xC0) >> 6)]);
        result.push_back(charset[text[cnt] & 0x3F]);

        break;
    }
  }

  return result;
}

}
}
