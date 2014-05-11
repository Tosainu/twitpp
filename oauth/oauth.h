#ifndef TWITPP_OAUTH_H
#define TWITPP_OAUTH_H

#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

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

    public:
      OAuth(asio::io_service& io_service, asio::ssl::context& context,
          const std::string& consumer_key, const std::string& consumer_secret);
      OAuth(asio::io_service& io_service, asio::ssl::context& context,
          const std::string& consumer_key, const std::string& consumer_secret,
          const std::string& oauth_token, const std::string& oauth_token_secret);

      int getAuthorizeUrl();
      std::string authorize_url_;
      int getOAuthToken(const std::string& pin);
  };

}

#endif
