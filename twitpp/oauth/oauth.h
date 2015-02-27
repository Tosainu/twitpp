#ifndef TWITPP_OAUTH_OAUTH_H
#define TWITPP_OAUTH_OAUTH_H

#include <map>
#include <memory>
#include <string>
#include "../net/response.h"
#include "account.h"

namespace twitpp {
namespace oauth {

class client {
public:
  template <typename T>
  client(T&& ac) : account_{std::forward<T>(ac)} {}

  net::response get(const std::string& url, const std::map<std::string, std::string>& parameters = decltype(parameters){});
  net::response post(const std::string& url, const std::map<std::string, std::string>& parameters = decltype(parameters){});

  void stream_get(const std::string& url, const net::response_handler& handler);
  void stream_get(const std::string& url, const std::map<std::string, std::string>& parameters,
                 const net::response_handler& handler);
  void stream_post(const std::string& url, const net::response_handler& handler);
  void stream_post(const std::string& url, const std::map<std::string, std::string>& parameters,
                  const net::response_handler& handler);

private:
  account account_;

  inline std::map<std::string, std::string> make_auth_param();
};

}
}

#endif
