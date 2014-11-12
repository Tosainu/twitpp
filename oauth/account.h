#ifndef TWITPP_OAUTH_ACCOUNT_H
#define TWITPP_OAUTH_ACCOUNT_H

#include <string>
#include "../utility/utility.h"

namespace twitpp {
namespace oauth {

class account {
public:
  account(const std::string& consumer_key, const std::string& consumer_secret);
  account(const std::string& api_url, const std::string& consumer_key, const std::string& consumer_secret);
  account(const std::string& consumer_key, const std::string& consumer_secret,
          const std::string& oauth_token, const std::string& oauth_token_secret);

  ~account();

  int get_authorize_url();
  int get_oauth_token(const std::string& pin);

  std::string authorize_url();

  std::string consumer_key() const;
  std::string consumer_secret() const;
  std::string oauth_token() const;
  std::string oauth_token_secret() const;

private:
  std::string api_url_;
  std::string authorize_url_;

  std::string consumer_key_;
  std::string consumer_secret_;
  std::string oauth_token_;
  std::string oauth_token_secret_;

  utility::random_str random_str_;
  utility::base64 base64_;
  utility::url url_;
  utility::hmac_sha1 hmac_sha1_;
};

}
}

#endif
