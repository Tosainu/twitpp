#ifndef TWITPP_NET_RESPONSE_H
#define TWITPP_NET_RESPONSE_H

namespace twitpp {
namespace net {

struct response {
  std::string http_version;
  int status_code;
  std::string status_message;
  std::map<std::string, std::string> header;
  std::string body;
};

using response_handler = std::function<void(response&)>;

}
}

#endif
