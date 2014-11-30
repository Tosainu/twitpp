#ifndef TWITPP_OAUTH_OAUTH_H
#define TWITPP_OAUTH_OAUTH_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "account.h"

namespace twitpp {
namespace oauth {

class client {
public:
  typedef boost::asio::io_service io_service;
  typedef boost::asio::ssl::context context;

  client(io_service& io_service, context& context, account& ac);

  ~client();

  void get(const std::string& host, const std::string& path, std::function<void(int&, std::string&)> handler);
  void get(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters,
           std::function<void(int&, std::string&)> handler);
  void post(const std::string& host, const std::string& path, std::function<void(int&, std::string&)> handler);
  void post(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters,
            std::function<void(int&, std::string&)> handler);

private:
  std::unique_ptr<account> account_;

  io_service& io_service_;
  context& context_;

  inline std::map<std::string, std::string> make_auth_param();
};

}
}

#endif
