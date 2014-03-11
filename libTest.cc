#include <iostream>
#include "twitpp.h"

int main() {
  std::string pin;

  // Set ConsumerKey
  twitpp session1("CONSUMER", "CONSUMER_SECRET");
  
  // Show AuthorizeUrl
  if(!session1.oauthGetAuthorizeUrl()) {
    std::cout << session1.authorizeUrl << std::endl;
  }
  else {
    return 1;
  }

  std::cin >> pin;
  // Get AccessToken
  session1.oauthGetAccessToken(pin);

  session1.update("Hello World!");

  return 0;
}
