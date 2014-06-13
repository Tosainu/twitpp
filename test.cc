#include <iostream>
#include <map>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "oauth/oauth.h"
#include "asioWrapper/asioWrapper.h"

int main() {
  namespace asio = boost::asio;

  asio::io_service io_service;
  asio::ssl::context ctx(asio::ssl::context::tlsv12);
  ctx.set_verify_mode(asio::ssl::verify_none);

  // set consumer key
  twitpp::OAuth oauth(io_service, ctx, "CONSUMER", "CONSUMER_SECRET");

  // get authorization url
  if(oauth.getAuthorizeUrl() == 0) {
    std::cout << "Access " << oauth.authorize_url_ << std::endl;
  }
  else {
    std::cerr << "Error!!" << std::endl;
    return 1;
  }

  // input pin
  std::string pin;
  std::cout << "Input PIN: ";
  std::cin >> pin;

  // get oauth token
  if(oauth.getOAuthToken(pin) == 0) {
    std::cout << "Authorization Succeeded !" << std::endl;
  }
  else {
    std::cerr << "Error!!" << std::endl;
    return 1;
  }

  // update
  std::map<std::string, std::string> pya;
  pya["status"] = "test";
  oauth.post("api.twitter.com", "/1.1/statuses/update.json", pya, [](int& status, std::string& text) {
    if (status != 200) {
      std::cerr << "Error!!" << std::endl;
      return;
    }

    std::cout << text << std::endl;
    text.assign("");
  });

  // userstream
  oauth.get("userstream.twitter.com", "/1.1/user.json", [](int& status, std::string& text) {
    if (status != 200) {
      std::cerr << "Error!!" << std::endl;
      return;
    }

    std::cout << text << std::endl;
    text.assign("");
  });

}
