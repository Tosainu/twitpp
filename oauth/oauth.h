#ifndef TWITPP_OAUTH_H
#define TWITPP_OAUTH_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "account.h"
#include "../utility/utility.h"

namespace twitpp {
namespace OAuth {

class Client {
public:
  typedef boost::asio::io_service io_service;
  typedef boost::asio::ssl::context context;

  Client(io_service& io_service, context& context, Account& account);

  ~Client();

  void get(const std::string& host, const std::string& path, std::function<void(int&, std::string&)> handler);
  void get(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters,
           std::function<void(int&, std::string&)> handler);
  void post(const std::string& host, const std::string& path, std::function<void(int&, std::string&)> handler);
  void post(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters,
            std::function<void(int&, std::string&)> handler);

private:
  std::unique_ptr<Account> account_;

  io_service& io_service_;
  context& context_;

  utility::RandomStr random_str_;
  utility::Base64 base64_;
  utility::Url url_;
  utility::HmacSha1 hmac_sha1_;
};

}
}

#endif
