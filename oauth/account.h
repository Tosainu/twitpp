#ifndef TWITPP_OAUTH_ACCOUNT_H
#define TWITPP_OAUTH_ACCOUNT_H

#include <string>
#include "../utility/utility.h"

namespace twitpp {
namespace OAuth {

class Account  {
public:
  Account(const std::string& consumer_key, const std::string& consumer_secret);
  Account(const std::string& api_url, const std::string& consumer_key, const std::string& consumer_secret);
  Account(const std::string& consumer_key, const std::string& consumer_secret,
          const std::string& oauth_token, const std::string& oauth_token_secret);

  ~Account();

  std::string get_authorize_url();
  void get_oauth_token(const std::string& pin);

  std::string get_consumer_key() const;
  std::string get_consumer_secret() const;
  std::string get_oauth_token() const;
  std::string get_oauth_token_secret() const;

private:
  std::string api_url_;

  std::string consumer_key_;
  std::string consumer_secret_;
  std::string oauth_token_;
  std::string oauth_token_secret_;

  utility::RandomStr random_str_;
  utility::Base64 base64_;
  utility::Url url_;
  utility::HmacSha1 hmac_sha1_;
};

}
}

#endif
