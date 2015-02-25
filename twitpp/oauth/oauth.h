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
  client(const account& ac);
  client(const account&& ac);

  net::response get(const std::string& url);
  net::response get(const std::string& url, const std::map<std::string, std::string>& parameters);
  net::response post(const std::string& url);
  net::response post(const std::string& url, const std::map<std::string, std::string>& parameters);

  void stream_get(const std::string& url, const net::response_handler& handler);
  void stream_get(const std::string& url, const std::map<std::string, std::string>& parameters,
                 const net::response_handler& handler);
  void stream_post(const std::string& url, const net::response_handler& handler);
  void stream_post(const std::string& url, const std::map<std::string, std::string>& parameters,
                  const net::response_handler& handler);

private:
  std::unique_ptr<account> account_;

  inline std::map<std::string, std::string> make_auth_param();
};

}
}

#endif
