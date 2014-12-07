#include <ctime>
#include <tuple>
#include <boost/lexical_cast.hpp>
#include "../net/async_client.h"
#include "../net/client.h"
#include "../util/util.h"
#include "oauth.h"

namespace twitpp {
namespace oauth {

client::client(account& ac)
    : account_(new account(ac)), io_service_(std::make_shared<boost::asio::io_service>()),
      context_(std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12)) {
  context_->set_verify_mode(boost::asio::ssl::verify_none);
}

client::~client() {}

net::response client::get(const std::string& url) {
  return get(url, {});
}

net::response client::get(const std::string& url, const std::map<std::string, std::string>& parameters) {
  auto auth_param = make_auth_param();
  auto parsed_url = util::url_parser(url);

  std::string query_str;
  if (!parameters.empty()) {
    auth_param.insert(parameters.begin(), parameters.end());

    // generate query string
    for (auto&& param : parameters) {
      query_str.append(param.first + "=" + util::url_encode(param.second) + "&");
    };
    query_str.erase(query_str.end() - 1, query_str.end());
  }

  // generate signature_base
  std::string signature_base;
  for (auto&& param : auth_param) {
    signature_base.append(param.first + "=" + util::url_encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("GET&" +
                        util::url_encode(std::get<1>(parsed_url) + "://" + std::get<2>(parsed_url) + std::get<3>(parsed_url)) +
                        "&" + util::url_encode(signature_base));

  // generate signing key
  std::string signing_key(util::url_encode(account_->consumer_secret()) + "&" + util::url_encode(account_->oauth_token_secret()));

  // set oauth_signature
  auth_param["oauth_signature"] = util::base64_encode(util::hmac_sha1_encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : auth_param) {
    authorization_header.append(param.first + "=\"" + util::url_encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // get
  try {
    net::client client(net::method::GET, query_str.empty() ? url : url + "?" + query_str);
    client.add_header(authorization_header);
    client.run();

    return client.response();
  } catch (std::exception& e) {
    throw e;
  }
}

net::response client::post(const std::string& url) {
  return post(url, {});
}

net::response client::post(const std::string& url, const std::map<std::string, std::string>& parameters) {
  auto auth_param = make_auth_param();
  auto parsed_url = util::url_parser(url);

  std::string query_str;
  if (!parameters.empty()) {
    auth_param.insert(parameters.begin(), parameters.end());

    // generate query string
    for (auto&& param : parameters) {
      query_str.append(param.first + "=" + util::url_encode(param.second) + "&");
    };
    query_str.erase(query_str.end() - 1, query_str.end());
  }

  // generate signature_base
  std::string signature_base;
  for (auto&& param : auth_param) {
    signature_base.append(param.first + "=" + util::url_encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("POST&" +
                        util::url_encode(std::get<1>(parsed_url) + "://" + std::get<2>(parsed_url) + std::get<3>(parsed_url)) +
                        "&" + util::url_encode(signature_base));

  // generate signing key
  std::string signing_key(util::url_encode(account_->consumer_secret()) + "&" + util::url_encode(account_->oauth_token_secret()));

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
    net::client client(net::method::POST, url);
    client.add_header(authorization_header);
    client.add_content(query_str);
    client.run();

    return client.response();
  } catch (std::exception& e) {
    throw e;
  }
}

void client::get(const std::string& host, const std::string& path, std::function<void(int&, std::string&)> handler) {
  auto auth_param = make_auth_param();

  // generate signature_base
  std::string signature_base;
  for (auto&& param : auth_param) {
    signature_base.append(param.first + "=" + util::url_encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("GET&" + util::url_encode("https://" + host + path) + "&" + util::url_encode(signature_base));

  // generate signing key
  std::string signing_key(util::url_encode(account_->consumer_secret()) + "&" + util::url_encode(account_->oauth_token_secret()));

  // set oauth_signature
  auth_param["oauth_signature"] = util::base64_encode(util::hmac_sha1_encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : auth_param) {
    authorization_header.append(param.first + "=\"" + util::url_encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // get
  net::async_client client(*io_service_, *context_, host, path);
  client.get(authorization_header, handler);
  io_service_->run();

  io_service_->reset();
}

void client::get(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters,
                 std::function<void(int&, std::string&)> handler) {
  auto auth_param = make_auth_param();
  auth_param.insert(parameters.begin(), parameters.end());

  // generate signature_base
  std::string signature_base;
  for (auto&& param : auth_param) {
    signature_base.append(param.first + "=" + util::url_encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("GET&" + util::url_encode("https://" + host + path) + "&" + util::url_encode(signature_base));

  // generate signing key
  std::string signing_key(util::url_encode(account_->consumer_secret()) + "&" + util::url_encode(account_->oauth_token_secret()));

  // set oauth_signature
  auth_param["oauth_signature"] = util::base64_encode(util::hmac_sha1_encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : auth_param) {
    authorization_header.append(param.first + "=\"" + util::url_encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // generate post_body
  std::string post_body;
  for (auto&& param : parameters) {
    post_body.append(param.first + "=" + util::url_encode(param.second) + "&");
  };
  post_body.erase(post_body.end() - 1, post_body.end());

  // get
  net::async_client client(*io_service_, *context_, host, path + "?" + post_body);
  client.get(authorization_header, handler);
  io_service_->run();

  io_service_->reset();
}

void client::post(const std::string& host, const std::string& path, std::function<void(int&, std::string&)> handler) {
  auto auth_param = make_auth_param();

  // generate signature_base
  std::string signature_base;
  for (auto&& param : auth_param) {
    signature_base.append(param.first + "=" + util::url_encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("POST&" + util::url_encode("https://" + host + path) + "&" + util::url_encode(signature_base));

  // generate signing key
  std::string signing_key(util::url_encode(account_->consumer_secret()) + "&" + util::url_encode(account_->oauth_token_secret()));

  // set oauth_signature
  auth_param["oauth_signature"] = util::base64_encode(util::hmac_sha1_encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : auth_param) {
    authorization_header.append(param.first + "=\"" + util::url_encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // post
  net::async_client client(*io_service_, *context_, host, path);
  client.post(authorization_header, "", handler);
  io_service_->run();

  io_service_->reset();
}

void client::post(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters,
                  std::function<void(int&, std::string&)> handler) {
  auto auth_param = make_auth_param();
  auth_param.insert(parameters.begin(), parameters.end());

  // generate signature_base
  std::string signature_base;
  for (auto&& param : auth_param) {
    signature_base.append(param.first + "=" + util::url_encode(param.second) + "&");
  }
  signature_base.erase(signature_base.end() - 1, signature_base.end());
  signature_base.assign("POST&" + util::url_encode("https://" + host + path) + "&" + util::url_encode(signature_base));

  // generate signing key
  std::string signing_key(util::url_encode(account_->consumer_secret()) + "&" + util::url_encode(account_->oauth_token_secret()));

  // set oauth_signature
  auth_param["oauth_signature"] = util::base64_encode(util::hmac_sha1_encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : auth_param) {
    authorization_header.append(param.first + "=\"" + util::url_encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // generate post_body
  std::string post_body;
  for (auto&& param : parameters) {
    post_body.append(param.first + "=" + util::url_encode(param.second) + "&");
  };
  post_body.erase(post_body.end() - 1, post_body.end());

  // post
  net::async_client client(*io_service_, *context_, host, path);
  client.post(authorization_header, post_body, handler);
  io_service_->run();

  io_service_->reset();
}

inline std::map<std::string, std::string> client::make_auth_param() {
  return std::map<std::string, std::string>{
    {"oauth_callback"         , "oob"},
    {"oauth_consumer_key"     , account_->consumer_key()},
    {"oauth_nonce"            , util::random_str(32)},
    {"oauth_signature_method" , "HMAC-SHA1"},
    {"oauth_timestamp"        , boost::lexical_cast<std::string>(std::time(0))},
    {"oauth_token"            , account_->oauth_token()},
    {"oauth_version"          , "1.0"}
  };
}

}
}
