#include <ctime>
#include <map>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/lexical_cast.hpp>
#include "account.h"
#include "../net/client.h"

namespace twitpp {
namespace oauth {

account::account(const std::string& consumer_key, const std::string& consumer_secret)
    : api_url_("api.twitter.com"), consumer_key_(consumer_key), consumer_secret_(consumer_secret) {}

account::account(const std::string& api_url, const std::string& consumer_key, const std::string& consumer_secret)
    : api_url_(api_url), consumer_key_(consumer_key), consumer_secret_(consumer_secret) {}

account::account(const std::string& consumer_key, const std::string& consumer_secret,
    const std::string& oauth_token, const std::string& oauth_token_secret)
    : consumer_key_(consumer_key), consumer_secret_(consumer_secret),
      oauth_token_(oauth_token), oauth_token_secret_(oauth_token_secret) {}

account::~account() {}

std::string account::get_authorize_url() {
  // TODO change to sync connection
  boost::asio::io_service io_service;
  boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12);
  ctx.set_verify_mode(boost::asio::ssl::verify_none);

  // set authorization_param
  std::map<std::string, std::string> authorization_param;
  authorization_param["oauth_callback"] = "oob";
  authorization_param["oauth_consumer_key"] = consumer_key_;
  authorization_param["oauth_nonce"] = random_str_(32);
  authorization_param["oauth_signature_method"] = "HMAC-SHA1";
  authorization_param["oauth_timestamp"] = boost::lexical_cast<std::string>(std::time(0));
  authorization_param["oauth_version"] = "1.0";

  // generate signature_base
  std::string signature_base;
  for (auto&& param : authorization_param) {
    signature_base.append(param.first + "=" + url_.encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("POST&" + url_.encode("https://" + api_url_ + "/oauth/request_token") + "&" +
                        url_.encode(signature_base));

  // generate signing key
  std::string signing_key(url_.encode(consumer_secret_) + "&");

  // set oauth_signature
  authorization_param["oauth_signature"] = base64_.encode(hmac_sha1_.encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : authorization_param) {
    authorization_header.append(param.first + "=\"" + url_.encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // post
  std::string authorize_url;
  net::async_client client(io_service, ctx, api_url_, "/oauth/request_token");
  client.post(authorization_header, "", [&](int& status, std::string& text) {
    if (status != 200) {
      return;
    }

    oauth_token_.assign(text);
    oauth_token_secret_.assign(text);
    oauth_token_.erase(oauth_token_.find("&oauth_token_secret=", 0));
    oauth_token_.erase(0, oauth_token_.find("token=", 0) + 6);
    oauth_token_secret_.erase(oauth_token_secret_.find("&oauth_callback_confirmed=", 0));
    oauth_token_secret_.erase(0, oauth_token_secret_.find("secret=", 0) + 7);

    authorize_url.assign("https://" + api_url_ + "/oauth/authorize\?oauth_token=" + oauth_token_);
  });
  io_service.run();

  io_service.reset();

  // if (client.response_.status_code != 200) {
  //   return 1;
  // }
  return authorize_url;
}

void account::get_oauth_token(const std::string& pin) {
  // TODO change to sync connection
  boost::asio::io_service io_service;
  boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12);
  ctx.set_verify_mode(boost::asio::ssl::verify_none);

  // set authorization_param
  std::map<std::string, std::string> authorization_param;
  authorization_param["oauth_callback"] = "oob";
  authorization_param["oauth_consumer_key"] = consumer_key_;
  authorization_param["oauth_nonce"] = random_str_(32);
  authorization_param["oauth_signature_method"] = "HMAC-SHA1";
  authorization_param["oauth_timestamp"] = boost::lexical_cast<std::string>(std::time(0));
  authorization_param["oauth_token"] = oauth_token_;
  authorization_param["oauth_verifier"] = pin;
  authorization_param["oauth_version"] = "1.0";

  // generate signature_base
  std::string signature_base;
  for (auto&& param : authorization_param) {
    signature_base.append(param.first + "=" + url_.encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("POST&" + url_.encode("https://" + api_url_ + "/oauth/access_token") + "&" +
                        url_.encode(signature_base));

  // generate signing key
  std::string signing_key(url_.encode(consumer_secret_) + "&" + url_.encode(oauth_token_secret_));

  // set oauth_signature
  authorization_param["oauth_signature"] = base64_.encode(hmac_sha1_.encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : authorization_param) {
    authorization_header.append(param.first + "=\"" + url_.encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // post
  net::async_client client(io_service, ctx, api_url_, "/oauth/access_token");
  client.post(authorization_header, "", [&](int& status, std::string& text) {
    if (status != 200) {
      return;
    }

    oauth_token_.assign(text);
    oauth_token_secret_.assign(text);
    oauth_token_.erase(oauth_token_.find("&oauth_token_secret=", 0));
    oauth_token_.erase(0, oauth_token_.find("token=", 0) + 6);
    oauth_token_secret_.erase(oauth_token_secret_.find("&user_id=", 0));
    oauth_token_secret_.erase(0, oauth_token_secret_.find("secret=", 0) + 7);
  });
  io_service.run();

  io_service.reset();

  // if (client.response_.status_code != 200) {
  //   return 1;
  // }
  // return 0;
}

std::string account::get_consumer_key() const {
  return consumer_key_;
}

std::string account::get_consumer_secret() const {
  return consumer_secret_;
}

std::string account::get_oauth_token() const {
  return oauth_token_;
}

std::string account::get_oauth_token_secret() const {
  return oauth_token_secret_;
}

}
}
