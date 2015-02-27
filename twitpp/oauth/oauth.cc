#include <ctime>
#include "../net/async_client.h"
#include "../net/client.h"
#include "../util/util.h"
#include "oauth.h"

namespace twitpp {
namespace oauth {

net::response client::get(const std::string& url, const std::map<std::string, std::string>& parameters) {
  auto auth_param = make_auth_param();

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
  signature_base.assign("GET&" + util::url_encode(url) + "&" + util::url_encode(signature_base));

  // generate signing key
  std::string signing_key(util::url_encode(account_.consumer_secret()) + "&" + util::url_encode(account_.oauth_token_secret()));

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
    throw;
  }
}

net::response client::post(const std::string& url, const std::map<std::string, std::string>& parameters) {
  auto auth_param = make_auth_param();

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
  signature_base.assign("POST&" + util::url_encode(url) + "&" + util::url_encode(signature_base));

  // generate signing key
  std::string signing_key(util::url_encode(account_.consumer_secret()) + "&" + util::url_encode(account_.oauth_token_secret()));

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
    throw;
  }
}

void client::stream_get(const std::string& url, const net::response_handler& handler) {
  stream_get(url, {}, handler);
}

void client::stream_get(const std::string& url, const std::map<std::string, std::string>& parameters,
                       const net::response_handler& handler) {
  auto auth_param = make_auth_param();

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
  signature_base.assign("GET&" + util::url_encode(url) + "&" + util::url_encode(signature_base));

  // generate signing key
  std::string signing_key(util::url_encode(account_.consumer_secret()) + "&" +
                          util::url_encode(account_.oauth_token_secret()));

  // set oauth_signature
  auth_param["oauth_signature"] = util::base64_encode(util::hmac_sha1_encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : auth_param) {
    authorization_header.append(param.first + "=\"" + util::url_encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // get
  net::async_client client(net::method::GET, url + "?" + query_str);
  client.add_header(authorization_header);
  client.run(handler);
}

void client::stream_post(const std::string& url, const net::response_handler& handler) {
  stream_post(url, {}, handler);
}

void client::stream_post(const std::string& url, const std::map<std::string, std::string>& parameters,
                        const net::response_handler& handler) {
  auto auth_param = make_auth_param();

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
  signature_base.assign("POST&" + util::url_encode(url) + "&" + util::url_encode(signature_base));

  // generate signing key
  std::string signing_key(util::url_encode(account_.consumer_secret()) + "&" +
                          util::url_encode(account_.oauth_token_secret()));

  // set oauth_signature
  auth_param["oauth_signature"] = util::base64_encode(util::hmac_sha1_encode(signing_key, signature_base));

  // generate authorization_header
  std::string authorization_header("Authorization: OAuth ");
  for (auto&& param : auth_param) {
    authorization_header.append(param.first + "=\"" + util::url_encode(param.second) + "\", ");
  }
  authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

  // post
  net::async_client client(net::method::POST, url);
  client.add_header(authorization_header);
  client.add_content(query_str);
  client.run(handler);
}

inline std::map<std::string, std::string> client::make_auth_param() {
  return std::map<std::string, std::string>{
    {"oauth_callback"         , "oob"},
    {"oauth_consumer_key"     , account_.consumer_key()},
    {"oauth_nonce"            , util::random_str(32)},
    {"oauth_signature_method" , "HMAC-SHA1"},
    {"oauth_timestamp"        , std::to_string(std::time(0))},
    {"oauth_token"            , account_.oauth_token()},
    {"oauth_version"          , "1.0"}
  };
}

}
}
