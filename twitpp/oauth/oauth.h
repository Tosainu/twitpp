#if !defined TWITPP_OAUTH_OAUTH_H
#define      TWITPP_OAUTH_OAUTH_H

#include <map>
#include <memory>
#include <string>
#include "../net/method.h"
#include "../net/response.h"
#include "account.h"

namespace twitpp {
namespace oauth {

using param_t = std::map<std::string, std::string>;

class client {
  account account_;

public:
  template <typename T>
  client(T&& ac) : account_{std::forward<T>(ac)} {}

  net::response get(const std::string& url, const param_t& param = param_t{});
  net::response post(const std::string& url, const param_t& param = param_t{});

  void stream_get(const std::string& url, const net::response_handler& handler);
  void stream_post(const std::string& url, const net::response_handler& handler);

  void stream_get(const std::string& url, const param_t& param, const net::response_handler& handler);
  void stream_post(const std::string& url, const param_t& param, const net::response_handler& handler);

private:
  net::response request(const net::method& method, const std::string& url, const param_t& param);
  void stream_request(net::method method, const std::string& url, const param_t& param,
                      const net::response_handler& handler);

  std::string make_query_str(const param_t& param);
  std::string make_auth_header(const param_t& param);

  inline param_t make_auth_param();
};

} // namespace oauth
} // namespace twitpp

#endif // TWITPP_OAUTH_OAUTH_H
