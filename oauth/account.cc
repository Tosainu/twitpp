#include <ctime>
#include <map>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/xpressive/xpressive.hpp>
#include "../net/client.h"
#include "../util/util.h"
#include "account.h"

namespace twitpp {
namespace oauth {

account::account(const std::string& consumer_key, const std::string& consumer_secret)
  : consumer_key_(consumer_key), consumer_secret_(consumer_secret) {}

account::account(const std::string& consumer_key, const std::string& consumer_secret,
                 const std::string& oauth_token, const std::string& oauth_token_secret)
  : consumer_key_(consumer_key), consumer_secret_(consumer_secret),
    oauth_token_(oauth_token), oauth_token_secret_(oauth_token_secret) {}

account::~account() {}

int account::get_authorize_url() {
  auto auth_param = make_auth_param();

  // generate signature_base
  std::string signature_base;
  for (auto&& param : auth_param) {
    signature_base.append(param.first + "=" + util::url_encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("POST&" + util::url_encode("https://api.twitter.com/oauth/request_token") + "&" +
                        util::url_encode(signature_base));

  // generate signing key
  std::string signing_key(util::url_encode(consumer_secret_) + "&");

  // set oauth_signature
  auth_param["oauth_signature"] = util::base64_encode(util::hmac_sha1_encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : auth_param) {
    authorization_header.append(param.first + "=\"" + util::url_encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // post
  try {
    net::client client(net::method::POST, "https://api.twitter.com/oauth/request_token");
    client.add_header(authorization_header);
    client.run();

    using namespace boost::xpressive;

    sregex regex_token = "oauth_token=" >> (s1 = +_w) >> '&'
                      >> "oauth_token_secret=" >> (s2 = +_w) >> '&'
                      >> "oauth_callback_confirmed=" >> (s3 = +_w);

    smatch token;
    if (regex_match(client.response().response_body, token, regex_token)) {
      oauth_token_.assign(token[1]);
      oauth_token_secret_.assign(token[2]);
      authorize_url_.assign("https://api.twitter.com/oauth/authorize\?oauth_token=" + token[1]);
      return 0;
    } else {
      authorize_url_.clear();
      return -1;
    }
  } catch (std::exception& e) {
    throw e;
  }
}

int account::get_oauth_token(const std::string& pin) {
  auto auth_param = make_auth_param();
  auth_param["oauth_verifier"] = pin;

  // generate signature_base
  std::string signature_base;
  for (auto&& param : auth_param) {
    signature_base.append(param.first + "=" + util::url_encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("POST&" + util::url_encode("https://api.twitter.com/oauth/access_token") + "&" +
                        util::url_encode(signature_base));

  // generate signing key
  std::string signing_key(util::url_encode(consumer_secret_) + "&" + util::url_encode(oauth_token_secret_));

  // set oauth_signature
  auth_param["oauth_signature"] = util::base64_encode(util::hmac_sha1_encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : auth_param) {
    authorization_header.append(param.first + "=\"" + util::url_encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // post
  try {
    net::client client(net::method::POST, "https://api.twitter.com/oauth/access_token");
    client.add_header(authorization_header);
    client.run();

    using namespace boost::xpressive;

    sregex regex_token = "oauth_token=" >> (s1 = +set['-' | alnum]) >> '&'
                      >> "oauth_token_secret=" >> (s2 = +_w) >> '&'
                      >> "user_id=" >> (s3 = +_w) >> '&'
                      >> "screen_name=" >> (s4 = +_w);

    smatch token;
    if (regex_match(client.response().response_body, token, regex_token)) {
      oauth_token_.assign(token[1]);
      oauth_token_secret_.assign(token[2]);
      return 0;
    } else {
      oauth_token_.clear();
      oauth_token_secret_.clear();
      return -1;
    }
  } catch (std::exception& e) {
    throw e;
  }
}

std::string account::authorize_url() {
  return authorize_url_;
}

std::string account::consumer_key() const {
  return consumer_key_;
}

std::string account::consumer_secret() const {
  return consumer_secret_;
}

std::string account::oauth_token() const {
  return oauth_token_;
}

std::string account::oauth_token_secret() const {
  return oauth_token_secret_;
}

inline std::map<std::string, std::string> account::make_auth_param() {
  std::map<std::string, std::string> auth_param{
    {"oauth_callback"         , "oob"},
    {"oauth_consumer_key"     , consumer_key_},
    {"oauth_nonce"            , util::random_str(32)},
    {"oauth_signature_method" , "HMAC-SHA1"},
    {"oauth_timestamp"        , boost::lexical_cast<std::string>(std::time(0))},
    {"oauth_version"          , "1.0"}
  };

  if (!oauth_token_.empty()) {
    auth_param["oauth_token"] = oauth_token_;
  }

  return auth_param;
}

}
}
