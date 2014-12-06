#ifndef TWITPP_OAUTH_OAUTH_H
#define TWITPP_OAUTH_OAUTH_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "../net/response.h"
#include "account.h"

namespace twitpp {
namespace oauth {

class client {
public:
  client(boost::asio::io_service& io_service, boost::asio::ssl::context& context, account& ac);

  ~client();

  net::response get(const std::string& url);
  net::response get(const std::string& url, const std::map<std::string, std::string>& parameters);

  net::response post(const std::string& url);
  net::response post(const std::string& url, const std::map<std::string, std::string>& parameters);

  void get(const std::string& host, const std::string& path, std::function<void(int&, std::string&)> handler);
  void get(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters,
           std::function<void(int&, std::string&)> handler);
  void post(const std::string& host, const std::string& path, std::function<void(int&, std::string&)> handler);
  void post(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters,
            std::function<void(int&, std::string&)> handler);

private:
  std::unique_ptr<account> account_;

  boost::asio::io_service& io_service_;
  boost::asio::ssl::context& context_;

  inline std::map<std::string, std::string> make_auth_param();
};

}
}

#endif
