#include <ctime>
#include <map>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/lexical_cast.hpp>
#include "account.h"
#include "oauth.h"
#include "../asioWrapper/asioWrapper.h"

namespace twitpp {
namespace OAuth {

Client::Client(io_service& io_service, context& context, Account& account)
    : account_(new Account(account)), io_service_(io_service), context_(context) {}

Client::~Client() {}

void Client::get(const std::string& host, const std::string& path, std::function<void(int&, std::string&)> handler) {
  // set authorization_param
  std::map<std::string, std::string> authorization_param;
  authorization_param["oauth_callback"] = "oob";
  authorization_param["oauth_consumer_key"] = account_->get_consumer_key();
  authorization_param["oauth_nonce"] = random_str_(32);
  authorization_param["oauth_signature_method"] = "HMAC-SHA1";
  authorization_param["oauth_timestamp"] = boost::lexical_cast<std::string>(std::time(0));
  authorization_param["oauth_token"] = account_->get_oauth_token();
  authorization_param["oauth_version"] = "1.0";

  // generate signature_base
  std::string signature_base;
  for (auto&& param : authorization_param) {
    signature_base.append(param.first + "=" + url_.encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("GET&" + url_.encode("https://" + host + path) + "&" + url_.encode(signature_base));

  // generate signing key
  std::string signing_key(url_.encode(account_->get_consumer_secret()) + "&" + url_.encode(account_->get_oauth_token_secret()));

  // set oauth_signature
  authorization_param["oauth_signature"] = base64_.encode(hmac_sha1_.encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : authorization_param) {
    authorization_header.append(param.first + "=\"" + url_.encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // get
  asioWrapper::Client client(io_service_, context_, host, path);
  client.get(authorization_header, handler);
  io_service_.run();

  io_service_.reset();
}

void Client::get(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters,
                 std::function<void(int&, std::string&)> handler) {
  // set authorization_param
  std::map<std::string, std::string> authorization_param;
  authorization_param["oauth_callback"] = "oob";
  authorization_param["oauth_consumer_key"] = account_->get_consumer_key();
  authorization_param["oauth_nonce"] = random_str_(32);
  authorization_param["oauth_signature_method"] = "HMAC-SHA1";
  authorization_param["oauth_timestamp"] = boost::lexical_cast<std::string>(std::time(0));
  authorization_param["oauth_token"] = account_->get_oauth_token();
  authorization_param["oauth_version"] = "1.0";
  authorization_param.insert(parameters.begin(), parameters.end());

  // generate signature_base
  std::string signature_base;
  for (auto&& param : authorization_param) {
    signature_base.append(param.first + "=" + url_.encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("GET&" + url_.encode("https://" + host + path) + "&" + url_.encode(signature_base));

  // generate signing key
  std::string signing_key(url_.encode(account_->get_consumer_secret()) + "&" + url_.encode(account_->get_oauth_token_secret()));

  // set oauth_signature
  authorization_param["oauth_signature"] = base64_.encode(hmac_sha1_.encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : authorization_param) {
    authorization_header.append(param.first + "=\"" + url_.encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // generate post_body
  std::string post_body;
  for (auto&& param : parameters) {
    post_body.append(param.first + "=" + url_.encode(param.second) + "&");
  };
  post_body.erase(post_body.end() - 1, post_body.end());

  // get
  asioWrapper::Client client(io_service_, context_, host, path + "?" + post_body);
  client.get(authorization_header, handler);
  io_service_.run();

  io_service_.reset();
}

void Client::post(const std::string& host, const std::string& path, std::function<void(int&, std::string&)> handler) {
  // set authorization_param
  std::map<std::string, std::string> authorization_param;
  authorization_param["oauth_callback"] = "oob";
  authorization_param["oauth_consumer_key"] = account_->get_consumer_key();
  authorization_param["oauth_nonce"] = random_str_(32);
  authorization_param["oauth_signature_method"] = "HMAC-SHA1";
  authorization_param["oauth_timestamp"] = boost::lexical_cast<std::string>(std::time(0));
  authorization_param["oauth_token"] = account_->get_oauth_token();
  authorization_param["oauth_version"] = "1.0";

  // generate signature_base
  std::string signature_base;
  for (auto&& param : authorization_param) {
    signature_base.append(param.first + "=" + url_.encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("POST&" + url_.encode("https://" + host + path) + "&" + url_.encode(signature_base));

  // generate signing key
  std::string signing_key(url_.encode(account_->get_consumer_secret()) + "&" + url_.encode(account_->get_oauth_token_secret()));

  // set oauth_signature
  authorization_param["oauth_signature"] = base64_.encode(hmac_sha1_.encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : authorization_param) {
    authorization_header.append(param.first + "=\"" + url_.encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // post
  asioWrapper::Client client(io_service_, context_, host, path);
  client.post(authorization_header, "", handler);
  io_service_.run();

  io_service_.reset();
}

void Client::post(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters,
                  std::function<void(int&, std::string&)> handler) {
  // set authorization_param
  std::map<std::string, std::string> authorization_param;
  authorization_param["oauth_callback"] = "oob";
  authorization_param["oauth_consumer_key"] = account_->get_consumer_key();
  authorization_param["oauth_nonce"] = random_str_(32);
  authorization_param["oauth_signature_method"] = "HMAC-SHA1";
  authorization_param["oauth_timestamp"] = boost::lexical_cast<std::string>(std::time(0));
  authorization_param["oauth_token"] = account_->get_oauth_token();
  authorization_param["oauth_version"] = "1.0";
  authorization_param.insert(parameters.begin(), parameters.end());

  // generate signature_base
  std::string signature_base;
  for (auto&& param : authorization_param) {
    signature_base.append(param.first + "=" + url_.encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("POST&" + url_.encode("https://" + host + path) + "&" + url_.encode(signature_base));

  // generate signing key
  std::string signing_key(url_.encode(account_->get_consumer_secret()) + "&" + url_.encode(account_->get_oauth_token_secret()));

  // set oauth_signature
  authorization_param["oauth_signature"] = base64_.encode(hmac_sha1_.encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : authorization_param) {
    authorization_header.append(param.first + "=\"" + url_.encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // generate post_body
  std::string post_body;
  for (auto&& param : parameters) {
    post_body.append(param.first + "=" + url_.encode(param.second) + "&");
  };
  post_body.erase(post_body.end() - 1, post_body.end());

  // post
  asioWrapper::Client client(io_service_, context_, host, path);
  client.post(authorization_header, post_body, handler);
  io_service_.run();

  io_service_.reset();
}

}
}
