#include <algorithm>
#include <ctime>
#include <map>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/lexical_cast.hpp>
#include "oauth.h"
#include "../asioWrapper/asioWrapper.h"

namespace twitpp {

  namespace asio = boost::asio;

  OAuth::OAuth(asio::io_service& io_service, asio::ssl::context& context,
      const std::string& consumer_key, const std::string& consumer_secret)
    : io_service_(io_service), context_(context),
    consumer_key_(consumer_key), consumer_secret_(consumer_secret) {
  }

  OAuth::OAuth(asio::io_service& io_service, asio::ssl::context& context,
      const std::string& consumer_key, const std::string& consumer_secret,
      const std::string& oauth_token, const std::string& oauth_token_secret)
    : io_service_(io_service), context_(context),
    consumer_key_(consumer_key), consumer_secret_(consumer_secret),
    oauth_token_(oauth_token), oauth_token_secret_(oauth_token_secret) {
  }

  int OAuth::getAuthorizeUrl() {
    // set parameters
    std::map<std::string, std::string> params;
    params["oauth_callback"] = "oob";
    params["oauth_consumer_key"] = consumer_key_;
    params["oauth_nonce"] = random_str_(32);
    params["oauth_signature_method"] = "HMAC-SHA1";
    params["oauth_timestamp"] = boost::lexical_cast<std::string>(std::time(0));
    params["oauth_version"] = "1.0";

    // generate signature_base
    std::string signature_base;
    std::for_each(params.begin(), params.end(), [&](std::pair<const std::string, std::string> param) {
        signature_base += param.first + "=" + url_.encode(param.second) + "&";
        });
    signature_base.erase(signature_base.end() - 1, signature_base.end());
    signature_base = "POST&" + url_.encode("https://api.twitter.com/oauth/request_token") + "&"
      + url_.encode(signature_base);

    // generate signing key
    std::string signing_key = url_.encode(consumer_secret_) + "&";

    // set oauth_signature
    params["oauth_signature"] = base64_.encode(hmac_sha1_.encode(signing_key, signature_base));

    // generate authorization_header
    std::string authorization_header = "Authorization: OAuth ";
    std::for_each(params.begin(), params.end(), [&](std::pair<const std::string, std::string> param) {
        authorization_header += param.first + "=\"" + url_.encode(param.second) + "\", ";
        });
    authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

    // post
    asioWrapper::Client client(io_service_, context_, "api.twitter.com", "/oauth/request_token");
    client.post(authorization_header, "", [&](std::string& text){
        oauth_token_ = text;
        oauth_token_secret_ = text;
        oauth_token_.erase(oauth_token_.find("&oauth_token_secret=", 0));
        oauth_token_.erase(0, oauth_token_.find("token=", 0) + 6);
        oauth_token_secret_.erase(oauth_token_secret_.find("&oauth_callback_confirmed=", 0));
        oauth_token_secret_.erase(0, oauth_token_secret_.find("secret=", 0) + 7);

        authorize_url_ = "https://api.twitter.com/oauth/authorize\?oauth_token=" + oauth_token_;
        });
    io_service_.run();

    io_service_.reset();

    if(client.response_.status_code != 200) {
      return 1;
    }

    return 0;
  }

  int OAuth::getOAuthToken(const std::string& pin) {
    // set parameters
    std::map<std::string, std::string> params;
    params["oauth_callback"] = "oob";
    params["oauth_consumer_key"] = consumer_key_;
    params["oauth_nonce"] = random_str_(32);
    params["oauth_signature_method"] = "HMAC-SHA1";
    params["oauth_timestamp"] = boost::lexical_cast<std::string>(std::time(0));
    params["oauth_token"] = oauth_token_;
    params["oauth_verifier"] = pin;
    params["oauth_version"] = "1.0";

    // generate signature_base
    std::string signature_base;
    std::for_each(params.begin(), params.end(), [&](std::pair<const std::string, std::string> param) {
        signature_base += param.first + "=" + url_.encode(param.second) + "&";
        });
    signature_base.erase(signature_base.end() - 1, signature_base.end());
    signature_base = "POST&" + url_.encode("https://api.twitter.com/oauth/access_token") + "&" + url_.encode(signature_base);

    // generate signing key
    std::string signing_key = url_.encode(consumer_secret_) + "&" + url_.encode(oauth_token_secret_);

    // set oauth_signature
    params["oauth_signature"] = base64_.encode(hmac_sha1_.encode(signing_key, signature_base));

    // generate authorization_header
    std::string authorization_header = "Authorization: OAuth ";
    std::for_each(params.begin(), params.end(), [&](std::pair<const std::string, std::string> param) {
        authorization_header += param.first + "=\"" + url_.encode(param.second) + "\", ";
        });
    authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

    // post
    asioWrapper::Client client(io_service_, context_, "api.twitter.com", "/oauth/access_token");
    client.post(authorization_header, "", [&](std::string& text){
        oauth_token_ = text;
        oauth_token_secret_ = text;
        oauth_token_.erase(oauth_token_.find("&oauth_token_secret=", 0));
        oauth_token_.erase(0, oauth_token_.find("token=", 0) + 6);
        oauth_token_secret_.erase(oauth_token_secret_.find("&user_id=", 0));
        oauth_token_secret_.erase(0, oauth_token_secret_.find("secret=", 0) + 7);
        });
    io_service_.run();

    io_service_.reset();

    if(client.response_.status_code != 200) {
      return 1;
    }

    return 0;
  }

  int OAuth::get(const std::string& host, const std::string& path, std::function<void(std::string&)> handler) {
    // set parameters
    std::map<std::string, std::string> params;
    params["oauth_callback"] = "oob";
    params["oauth_consumer_key"] = consumer_key_;
    params["oauth_nonce"] = random_str_(32);
    params["oauth_signature_method"] = "HMAC-SHA1";
    params["oauth_timestamp"] = boost::lexical_cast<std::string>(std::time(0));
    params["oauth_token"] = oauth_token_;
    params["oauth_version"] = "1.0";

    // generate signature_base
    std::string signature_base;
    std::for_each(params.begin(), params.end(), [&](std::pair<const std::string, std::string> param) {
        signature_base += param.first + "=" + url_.encode(param.second) + "&";
        });
    signature_base.erase(signature_base.end() - 1, signature_base.end());
    signature_base = "GET&" + url_.encode("https://" + host + path) + "&" + url_.encode(signature_base);

    // generate signing key
    std::string signing_key = url_.encode(consumer_secret_) + "&" + url_.encode(oauth_token_secret_);

    // set oauth_signature
    params["oauth_signature"] = base64_.encode(hmac_sha1_.encode(signing_key, signature_base));

    // generate authorization_header
    std::string authorization_header = "Authorization: OAuth ";
    std::for_each(params.begin(), params.end(), [&](std::pair<const std::string, std::string> param) {
        authorization_header += param.first + "=\"" + url_.encode(param.second) + "\", ";
        });
    authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

    // get
    asioWrapper::Client client(io_service_, context_, host, path);
    client.get(authorization_header, handler);
    io_service_.run();

    io_service_.reset();

    if(client.response_.status_code != 200) {
      return 1;
    }

    return 0;
  }

  int OAuth::get(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters, std::function<void(std::string&)> handler) {
    // set parameters
    std::map<std::string, std::string> params;
    params["oauth_callback"] = "oob";
    params["oauth_consumer_key"] = consumer_key_;
    params["oauth_nonce"] = random_str_(32);
    params["oauth_signature_method"] = "HMAC-SHA1";
    params["oauth_timestamp"] = boost::lexical_cast<std::string>(std::time(0));
    params["oauth_token"] = oauth_token_;
    params["oauth_version"] = "1.0";
    params.insert(parameters.begin(), parameters.end());

    // generate post body
    std::string post_body;
    std::for_each(parameters.begin(), parameters.end(), [&](std::pair<const std::string, std::string> param) {
        post_body += param.first + "=" + url_.encode(param.second) + "&";
        });
    post_body.erase(post_body.end() - 1, post_body.end());

    // generate signature_base
    std::string signature_base;
    std::for_each(params.begin(), params.end(), [&](std::pair<const std::string, std::string> param) {
        signature_base += param.first + "=" + url_.encode(param.second) + "&";
        });
    signature_base.erase(signature_base.end() - 1, signature_base.end());
    signature_base = "GET&" + url_.encode("https://" + host + path) + "&" + url_.encode(signature_base);

    // generate signing key
    std::string signing_key = url_.encode(consumer_secret_) + "&" + url_.encode(oauth_token_secret_);

    // set oauth_signature
    params["oauth_signature"] = base64_.encode(hmac_sha1_.encode(signing_key, signature_base));

    // generate authorization_header
    std::string authorization_header = "Authorization: OAuth ";
    std::for_each(params.begin(), params.end(), [&](std::pair<const std::string, std::string> param) {
        authorization_header += param.first + "=\"" + url_.encode(param.second) + "\", ";
        });
    authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

    // get
    asioWrapper::Client client(io_service_, context_, host, path + "?" + post_body);
    client.get(authorization_header, handler);
    io_service_.run();

    io_service_.reset();

    if(client.response_.status_code != 200) {
      return 1;
    }

    return 0;
  }

  int OAuth::post(const std::string& host, const std::string& path, std::function<void(std::string&)> handler) {
    // set parameters
    std::map<std::string, std::string> params;
    params["oauth_callback"] = "oob";
    params["oauth_consumer_key"] = consumer_key_;
    params["oauth_nonce"] = random_str_(32);
    params["oauth_signature_method"] = "HMAC-SHA1";
    params["oauth_timestamp"] = boost::lexical_cast<std::string>(std::time(0));
    params["oauth_token"] = oauth_token_;
    params["oauth_version"] = "1.0";

    // generate signature_base
    std::string signature_base;
    std::for_each(params.begin(), params.end(), [&](std::pair<const std::string, std::string> param) {
        signature_base += param.first + "=" + url_.encode(param.second) + "&";
        });
    signature_base.erase(signature_base.end() - 1, signature_base.end());
    signature_base = "POST&" + url_.encode("https://" + host + path) + "&" + url_.encode(signature_base);

    // generate signing key
    std::string signing_key = url_.encode(consumer_secret_) + "&" + url_.encode(oauth_token_secret_);

    // set oauth_signature
    params["oauth_signature"] = base64_.encode(hmac_sha1_.encode(signing_key, signature_base));

    // generate authorization_header
    std::string authorization_header = "Authorization: OAuth ";
    std::for_each(params.begin(), params.end(), [&](std::pair<const std::string, std::string> param) {
        authorization_header += param.first + "=\"" + url_.encode(param.second) + "\", ";
        });
    authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

    // post
    asioWrapper::Client client(io_service_, context_, host, path);
    client.post(authorization_header, "", handler);
    io_service_.run();

    io_service_.reset();

    if(client.response_.status_code != 200) {
      std::cout << client.response_.status_code << " ";
      return 1;
    }

    return 0;
  }


  int OAuth::post(const std::string& host, const std::string& path, const std::map<std::string, std::string> parameters, std::function<void(std::string&)> handler) {
    // set parameters
    std::map<std::string, std::string> params;
    params["oauth_callback"] = "oob";
    params["oauth_consumer_key"] = consumer_key_;
    params["oauth_nonce"] = random_str_(32);
    params["oauth_signature_method"] = "HMAC-SHA1";
    params["oauth_timestamp"] = boost::lexical_cast<std::string>(std::time(0));
    params["oauth_token"] = oauth_token_;
    params["oauth_version"] = "1.0";
    params.insert(parameters.begin(), parameters.end());

    // generate signature_base
    std::string signature_base;
    std::for_each(params.begin(), params.end(), [&](std::pair<const std::string, std::string> param) {
        signature_base += param.first + "=" + url_.encode(param.second) + "&";
        });
    signature_base.erase(signature_base.end() - 1, signature_base.end());
    signature_base = "POST&" + url_.encode("https://" + host + path) + "&" + url_.encode(signature_base);

    // generate signing key
    std::string signing_key = url_.encode(consumer_secret_) + "&" + url_.encode(oauth_token_secret_);

    // set oauth_signature
    params["oauth_signature"] = base64_.encode(hmac_sha1_.encode(signing_key, signature_base));

    // generate authorization_header
    std::string authorization_header = "Authorization: OAuth ";
    std::for_each(params.begin(), params.end(), [&](std::pair<const std::string, std::string> param) {
        authorization_header += param.first + "=\"" + url_.encode(param.second) + "\", ";
        });
    authorization_header.erase(authorization_header.end() - 2, authorization_header.end());

    // generate post body
    std::string post_body;
    std::for_each(parameters.begin(), parameters.end(), [&](std::pair<const std::string, std::string> param) {
        post_body += param.first + "=" + url_.encode(param.second) + "&";
        });
    post_body.erase(post_body.end() - 1, post_body.end());

    // post
    asioWrapper::Client client(io_service_, context_, host, path);
    client.post(authorization_header, post_body, handler);
    io_service_.run();

    io_service_.reset();

    if(client.response_.status_code != 200) {
      std::cout << client.response_.status_code << " ";
      return 1;
    }

    return 0;
  }

}
