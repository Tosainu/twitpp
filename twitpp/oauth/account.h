#ifndef TWITPP_OAUTH_ACCOUNT_H
#define TWITPP_OAUTH_ACCOUNT_H

#include <map>
#include <string>

namespace twitpp {
namespace oauth {

class account {
public:
  account(const std::string& consumer_key, const std::string& consumer_secret);
  account(const std::string& consumer_key, const std::string& consumer_secret,
          const std::string& oauth_token, const std::string& oauth_token_secret);

  int get_authorize_url();
  int get_oauth_token(const std::string& pin);

  std::string authorize_url();

  std::string consumer_key() const;
  std::string consumer_secret() const;
  std::string oauth_token() const;
  std::string oauth_token_secret() const;

private:
  std::string authorize_url_;

  std::string consumer_key_;
  std::string consumer_secret_;
  std::string oauth_token_;
  std::string oauth_token_secret_;

  inline std::map<std::string, std::string> make_auth_param();
};

}
}

#endif
