#ifndef TWITPP_OAUTH_H
#define TWITPP_OAUTH_H

#include <functional>
#include <map>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "../utility/utility.h"

namespace twitpp {

namespace asio = boost::asio;

class OAuth {
private:
  asio::io_service& io_service_;
  asio::ssl::context& context_;

  std::string consumer_key_;
  std::string consumer_secret_;
  std::string oauth_token_;
  std::string oauth_token_secret_;

  utility::RandomStr random_str_;
  utility::Base64 base64_;
  utility::Url url_;
  utility::HmacSha1 hmac_sha1_;

public:
  OAuth(asio::io_service& io_service, asio::ssl::context& context, const std::string& consumer_key,
        const std::string& consumer_secret);
  OAuth(asio::io_service& io_service, asio::ssl::context& context, const std::string& consumer_key,
        const std::string& consumer_secret, const std::string& oauth_token, const std::string& oauth_token_secret);

  int getAuthorizeUrl();
  std::string authorize_url_;
  int getOAuthToken(const std::string& pin);

  int get(const std::string& host, const std::string& path, std::function<void(std::string&)> handler);
  int get(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters,
          std::function<void(std::string&)> handler);
  int post(const std::string& host, const std::string& path, std::function<void(std::string&)> handler);
  int post(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters,
           std::function<void(std::string&)> handler);
};

}

#endif
