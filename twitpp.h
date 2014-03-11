#ifndef _INC_TWITPP
#define _INC_TWITPP

#include <iostream>
#include <string>

class twitpp {
  // Debug
  bool curlVerbose = 0;

  std::string requestTokenUrl;
  std::string accessTokenUrl;
  std::string consumerKey;
  std::string consumerSecret;
  std::string oauthToken;
  std::string oauthTokenSecret;
  std::string userId;
  std::string screenName;

  std::string signatureBaseString;
  std::string signingKey;
  std::string oauthSignature;
  std::string oauthHeader;

  std::string result;

  public:
  twitpp(const std::string& ck, const std::string& csk);
  twitpp(const std::string& ck, const std::string& csk, const std::string& ot, const std::string& ots);

  int oauthGetAuthorizeUrl();
  std::string authorizeUrl;
  int oauthGetAccessToken(const std::string& pin);

  int oauthGetRequest();
  int oauthPostRequest(const std::string& url, const std::string& param);

  std::string update(const std::string& text);
};

#endif
