#include <ctime>
#include <boost/xpressive/xpressive.hpp>
#include "../net/client.hh"
#include "../util/util.hh"
#include "account.hh"

namespace twitpp {
namespace oauth {

account::account(const std::string& ck, const std::string& cs, const std::string& ot, const std::string& os)
    : consumer_key_(ck), consumer_secret_(cs), oauth_token_(ot), oauth_token_secret_(os) {}

int account::get_authorize_url() {
  auto auth_param = make_auth_param();

  // generate signature_base
  std::string signature_base("POST&" + util::url_encode("https://api.twitter.com/oauth/request_token") + "&" +
                             util::url_encode(make_query_str(auth_param)));

  // generate signing key
  std::string signing_key(util::url_encode(consumer_secret_) + "&");

  // set oauth_signature
  auth_param["oauth_signature"] = util::hmac_sha1_encode(signing_key, signature_base);

  // post
  try {
    net::client client(net::method::POST, "https://api.twitter.com/oauth/request_token", make_auth_header(auth_param), "");
    client.run();

    using namespace boost::xpressive;

    sregex regex_token = "oauth_token=" >> (s1 = +_w) >> '&'
                      >> "oauth_token_secret=" >> (s2 = +_w) >> '&'
                      >> "oauth_callback_confirmed=" >> (s3 = +_w);

    smatch token;
    if (regex_match(client.response().body, token, regex_token)) {
      oauth_token_.assign(token.str(1));
      oauth_token_secret_.assign(token.str(2));
      authorize_url_.assign("https://api.twitter.com/oauth/authorize\?oauth_token=" + token.str(1));
      return 0;
    } else {
      authorize_url_.clear();
      return -1;
    }
  } catch (std::exception& e) {
    throw;
  }
}

int account::get_oauth_token(const std::string& pin) {
  auto auth_param = make_auth_param();
  auth_param["oauth_verifier"] = pin;

  // generate signature_base
  std::string signature_base("POST&" + util::url_encode("https://api.twitter.com/oauth/access_token") + "&" +
                             util::url_encode(make_query_str(auth_param)));

  // generate signing key
  std::string signing_key(util::url_encode(consumer_secret_) + "&" + util::url_encode(oauth_token_secret_));

  // set oauth_signature
  auth_param["oauth_signature"] = util::hmac_sha1_encode(signing_key, signature_base);

  // post
  try {
    net::client client(net::method::POST, "https://api.twitter.com/oauth/access_token", make_auth_header(auth_param), "");
    client.run();

    using namespace boost::xpressive;

    sregex regex_token = "oauth_token=" >> (s1 = +set['-' | alnum]) >> '&'
                      >> "oauth_token_secret=" >> (s2 = +_w) >> '&'
                      >> "user_id=" >> (s3 = +_w) >> '&'
                      >> "screen_name=" >> (s4 = +_w);

    smatch token;
    if (regex_match(client.response().body, token, regex_token)) {
      oauth_token_.assign(token.str(1));
      oauth_token_secret_.assign(token.str(2));
      return 0;
    } else {
      oauth_token_.clear();
      oauth_token_secret_.clear();
      return -1;
    }
  } catch (std::exception& e) {
    throw;
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

std::string account::make_query_str(const std::map<std::string, std::string>& param) {
  std::string s;

  for (auto&& p : param) {
    s.append(p.first + "=" + util::url_encode(p.second) + "&");
  };

  return s.erase(s.length() - 1);
}

std::string account::make_auth_header(const std::map<std::string, std::string>& param) {
  std::string s("Authorization: OAuth ");

  for (auto&& p : param) {
    s.append(p.first + "=\"" + util::url_encode(p.second) + "\", ");
  };

  return s.erase(s.length() - 2);
}

inline std::map<std::string, std::string> account::make_auth_param() {
  std::map<std::string, std::string> auth_param{
    {"oauth_callback"         , "oob"},
    {"oauth_consumer_key"     , consumer_key_},
    {"oauth_nonce"            , util::random_str(32)},
    {"oauth_signature_method" , "HMAC-SHA1"},
    {"oauth_timestamp"        , std::to_string(std::time(0))},
    {"oauth_version"          , "1.0"}
  };

  if (!oauth_token_.empty()) {
    auth_param["oauth_token"] = oauth_token_;
  }

  return auth_param;
}

} // namespace oauth
} // namespace twitpp
