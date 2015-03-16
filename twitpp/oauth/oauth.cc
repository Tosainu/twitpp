#include <ctime>
#include "../net/async_client.hh"
#include "../net/client.hh"
#include "../util/util.hh"
#include "oauth.hh"

namespace twitpp {
namespace oauth {

net::response client::get(const std::string& url, const param_t& param) {
  return request(net::method::GET, url, param);
}
net::response client::post(const std::string& url, const param_t& param) {
  return request(net::method::POST, url, param);
}

void client::stream_get(const std::string& url, const net::response_handler& handler) {
  stream_request(net::method::GET, url, param_t{}, handler);
}
void client::stream_post(const std::string& url, const net::response_handler& handler) {
  stream_request(net::method::POST, url, param_t{}, handler);
}

void client::stream_get(const std::string& url, const param_t& param, const net::response_handler& handler) {
  stream_request(net::method::GET, url, param, handler);
}
void client::stream_post(const std::string& url, const param_t& param, const net::response_handler& handler) {
  stream_request(net::method::POST, url, param, handler);
}

net::response client::request(const net::method& method, const std::string& url, const param_t& param) {
  auto auth_param = make_auth_param();

  // generate query string
  std::string query_str;
  if (!param.empty()) {
    query_str = make_query_str(param);
    auth_param.insert(param.begin(), param.end());
  }

  // generate signature_base
  std::string signature_base;
  switch (method) {
    case net::method::GET:
      signature_base.assign("GET&" + util::url_encode(url) + "&" + util::url_encode(make_query_str(auth_param)));
      break;
    case net::method::POST:
      signature_base.assign("POST&" + util::url_encode(url) + "&" + util::url_encode(make_query_str(auth_param)));
      break;
  }

  // generate signing key
  std::string signing_key(util::url_encode(account_.consumer_secret()) + "&" + util::url_encode(account_.oauth_token_secret()));

  // set oauth_signature
  auth_param["oauth_signature"] = util::hmac_sha1_encode(signing_key, signature_base);

  try {
    net::client client(method, url, make_auth_header(auth_param), query_str);
    client.run();
    return client.response();
  } catch (std::exception& e) {
    throw;
  }
}

void client::stream_request(net::method method, const std::string& url, const param_t& param,
                            const net::response_handler& handler) {
  auto auth_param = make_auth_param();

  // generate query string
  std::string query_str;
  if (!param.empty()) {
    query_str = make_query_str(param);
    auth_param.insert(param.begin(), param.end());
  }

  // generate signature_base
  std::string signature_base;
  switch (method) {
    case net::method::GET:
      signature_base.assign("GET&" + util::url_encode(url) + "&" + util::url_encode(make_query_str(auth_param)));
      break;
    case net::method::POST:
      signature_base.assign("POST&" + util::url_encode(url) + "&" + util::url_encode(make_query_str(auth_param)));
      break;
  }

  // generate signing key
  std::string signing_key(util::url_encode(account_.consumer_secret()) + "&" + util::url_encode(account_.oauth_token_secret()));

  // set oauth_signature
  auth_param["oauth_signature"] = util::hmac_sha1_encode(signing_key, signature_base);

  net::async_client client(method, url, make_auth_header(auth_param), query_str);
  client.run(handler);
}

std::string client::make_query_str(const param_t& param) {
  std::string s;

  for (auto&& p : param) {
    s.append(p.first + "=" + util::url_encode(p.second) + "&");
  };

  return s.erase(s.length() - 1);
}

std::string client::make_auth_header(const param_t& param) {
  std::string s("Authorization: OAuth ");

  for (auto&& p : param) {
    s.append(p.first + "=\"" + util::url_encode(p.second) + "\", ");
  };

  return s.erase(s.length() - 2);
}

inline param_t client::make_auth_param() {
  return param_t{
    {"oauth_callback"         , "oob"},
    {"oauth_consumer_key"     , account_.consumer_key()},
    {"oauth_nonce"            , util::random_str(32)},
    {"oauth_signature_method" , "HMAC-SHA1"},
    {"oauth_timestamp"        , std::to_string(std::time(0))},
    {"oauth_token"            , account_.oauth_token()},
    {"oauth_version"          , "1.0"}
  };
}

} // namespace oauth
} // namespace twitpp
