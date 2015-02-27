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
  using param_t = std::map<std::string, std::string>;

public:
  template <typename T>
  client(T&& ac) : account_{std::forward<T>(ac)} {}

  net::response get(const std::string& url, const param_t& parameters = param_t{});
  net::response post(const std::string& url, const param_t& parameters = param_t{});

  void stream_get(const std::string& url, const net::response_handler& handler);
  void stream_get(const std::string& url, const param_t& parameters,
                 const net::response_handler& handler);
  void stream_post(const std::string& url, const net::response_handler& handler);
  void stream_post(const std::string& url, const param_t& parameters,
                  const net::response_handler& handler);

private:
  account account_;

  inline std::map<std::string, std::string> make_auth_param();
};

}
}

#endif
