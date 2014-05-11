#include "twitpp.h"
#include <iostream>
#include <string>
#include <curl/curl.h>
#include "stringTool.h"

size_t write_to_string(void *ptr, size_t size, size_t count, void *stream) {
  ((std::string*)stream)->append((char*)ptr, 0, size*count);
  return size*count;
}

twitpp::twitpp(const std::string& ck, const std::string& csk) {
  consumerKey = ck;
  consumerSecret = csk;
  oauthToken = "";
  oauthTokenSecret = "";
}

twitpp::twitpp(const std::string& ck, const std::string& csk, const std::string& ot, const std::string& ots) {
  consumerKey = ck;
  consumerSecret = csk;
  oauthToken = ot;
  oauthTokenSecret = ots;
}

int twitpp::oauthGetAuthorizeUrl() {
  // OAuth nonce
  srand(time(NULL));
  const std::string oauth_nonce = md5(intToString(rand()));
  // Method
  const std::string method = "POST";
  // Timestamp
  time_t t = time(0);

  // Generate Signature base String
  signatureBaseString = "oauth_callback=" + rawurlencode("oob")
    + "&oauth_consumer_key=" + rawurlencode(consumerKey)
    + "&oauth_nonce=" + rawurlencode(oauth_nonce)
    + "&oauth_signature_method=" + rawurlencode("HMAC-SHA1")
    + "&oauth_timestamp=" + rawurlencode(intToString(t))
    + "&oauth_version=" + rawurlencode("1.0");

  signatureBaseString = method + "&" + rawurlencode("https://api.twitter.com/oauth/request_token") + "&" + rawurlencode(signatureBaseString);

  // Generate Signing key
  signingKey = rawurlencode(consumerSecret) + "&";

  // Generate oauth_signature
  oauthSignature = base64_encode(hmac_sha1(signingKey, signatureBaseString));

  // Generate Request_Token URL
  oauthHeader = "Authorization: OAuth oauth_callback=\"" + rawurlencode("oob")
    + "\",oauth_consumer_key=\"" + rawurlencode(consumerKey)
    + "\",oauth_nonce=\"" + rawurlencode(oauth_nonce)
    + "\",oauth_signature_method=\"" + rawurlencode("HMAC-SHA1")
    + "\",oauth_timestamp=\"" + rawurlencode(intToString(t))
    + "\",oauth_version=\"" + rawurlencode("1.0")
    + "\",oauth_signature=\"" + rawurlencode(oauthSignature);

  oauthHeader.erase(oauthHeader.size() - 2);
  oauthHeader += "\"";

  // cURL Init
  CURL *curl;
  CURLcode ret;
  struct curl_slist *headers = NULL;
  std::string chunk;

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();

  if(curl == NULL) {
    std::cerr << "curl_easy_init() failed" << std::endl;
    return 1;
  }

  headers = curl_slist_append(headers, "Expect:");
  headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
  headers = curl_slist_append(headers, oauthHeader.c_str());

  curl_easy_setopt(curl, CURLOPT_VERBOSE, curlVerbose);
  curl_easy_setopt(curl, CURLOPT_URL, "https://api.twitter.com/oauth/request_token");
  curl_easy_setopt(curl, CURLOPT_POST, 1);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER , headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (std::string*)&chunk);

  // Get OAuth Token
  ret = curl_easy_perform(curl);

  if(ret != CURLE_OK) {
    std::cerr << "curl_easy_perform() failed." << std::endl;
    return 1;
  }

  if(chunk.find("oauth_token") == std::string::npos) {
    std::cerr << chunk << std::endl;
    return 1;
  }

  oauthToken = chunk;
  oauthTokenSecret = chunk;
  oauthToken.erase(oauthToken.find("&oauth_token_secret=", 0));
  oauthToken.erase(0, oauthToken.find("token=", 0) + 6);
  oauthTokenSecret.erase(oauthTokenSecret.find("&oauth_callback_confirmed=", 0));
  oauthTokenSecret.erase(0, oauthTokenSecret.find("secret=", 0) + 7);
  authorizeUrl = "https://api.twitter.com/oauth/authorize\?oauth_token=" + oauthToken;

  return 0;
}

int twitpp::oauthGetAccessToken(const std::string& pin) {
  // OAuth nonce
  srand(time(NULL));
  const std::string oauth_nonce = md5(intToString(rand()));
  // Method
  const std::string method = "POST";
  // Timestamp
  time_t t = time(0);

  // reGenerate Signature base String
  signatureBaseString = "oauth_consumer_key=" + rawurlencode(consumerKey)
    + "&oauth_nonce=" + rawurlencode(oauth_nonce) 
    + "&oauth_signature_method=" + rawurlencode("HMAC-SHA1")
    + "&oauth_timestamp=" + rawurlencode(intToString(t))
    + "&oauth_token=" + rawurlencode(oauthToken)
    + "&oauth_verifier=" + rawurlencode(pin)
    + "&oauth_version=" + rawurlencode("1.0");
  signatureBaseString = method + "&" + rawurlencode("https://api.twitter.com/oauth/request_token") + "&" + rawurlencode(signatureBaseString);

  // reGenerate Signing key
  signingKey = rawurlencode(consumerSecret) + "&" + rawurlencode(oauthTokenSecret);

  // reGenerate oauth_signature
  oauthSignature = base64_encode(hmac_sha1(signingKey, signatureBaseString));

  // reGenerate Request_Token URL
  oauthHeader = "Authorization: OAuth oauth_consumer_key=\"" + rawurlencode(consumerKey)
    + "\",oauth_nonce=\"" + rawurlencode(oauth_nonce) 
    + "\",oauth_signature_method=\"" + rawurlencode("HMAC-SHA1")
    + "\",oauth_timestamp=\"" + rawurlencode(intToString(t))
    + "\",oauth_token=\"" + rawurlencode(oauthToken)
    + "\",oauth_verifier=\"" + rawurlencode(pin)
    + "\",oauth_version=\"" + rawurlencode("1.0")
    + "\",oauth_signature=\"" + rawurlencode(oauthSignature);
  oauthHeader.erase(oauthHeader.size() - 2);
  oauthHeader += "\"";

  // cURL Init
  CURL *curl;
  CURLcode ret;
  struct curl_slist *headers = NULL;
  std::string chunk;

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();

  if(curl == NULL) {
    std::cerr << "curl_easy_init() failed" << std::endl;
    return 1;
  }

  headers = curl_slist_append(headers, "Expect:");
  headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
  headers = curl_slist_append(headers, oauthHeader.c_str());

  curl_easy_setopt(curl, CURLOPT_VERBOSE, curlVerbose);
  curl_easy_setopt(curl, CURLOPT_URL, "https://api.twitter.com/oauth/access_token");
  curl_easy_setopt(curl, CURLOPT_POST, 1);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER , headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (std::string*)&chunk);

  // Get OAuth Token
  ret = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);

  if(ret != CURLE_OK) {
    std::cerr << "curl_easy_perform() failed." << std::endl;
    return 1;
  }

  if(chunk.find("screen_name", 0) == std::string::npos) {
    std::cerr << chunk << std::endl;
    return 1;
  }

  oauthToken = chunk;
  oauthTokenSecret = chunk;
  userId = chunk;
  screenName = chunk;
  oauthToken.erase(oauthToken.find("&oauth_token_secret=", 0));
  oauthToken.erase(0, oauthToken.find("token=", 0) + 6);
  oauthTokenSecret.erase(oauthTokenSecret.find("&user_id=", 0));
  oauthTokenSecret.erase(0, oauthTokenSecret.find("secret=", 0) + 7);
  userId.erase(userId.find("&screen_name=", 0));
  userId.erase(0, userId.find("&user_id=", 0) + 9);
  screenName.erase(0, screenName.find("screen_name=", 0) + 12);

  return 0;
}

void twitpp::showToken() {
  std::cout << oauthToken << std::endl;
  std::cout << oauthTokenSecret << std::endl;
}

int twitpp::oauthGetRequest() {
  return 0;
}

int twitpp::oauthPostRequest(const std::string& url, const std::string& param) {
  // OAuth nonce
  srand(time(NULL));
  const std::string oauth_nonce = md5(intToString(rand()));
  // Method
  const std::string method = "POST";
  // Timestamp
  time_t t = time(0);

  // Generate Signature base String
  signatureBaseString = "oauth_consumer_key=" + rawurlencode(consumerKey)
    + "&oauth_nonce=" + rawurlencode(oauth_nonce) 
    + "&oauth_signature_method=" + rawurlencode("HMAC-SHA1")
    + "&oauth_timestamp=" + rawurlencode(intToString(t))
    + "&oauth_token=" + rawurlencode(oauthToken)
    + "&oauth_version=" + rawurlencode("1.0")
    + "&" + param;
  signatureBaseString = method + "&" + rawurlencode(url) + "&" + rawurlencode(signatureBaseString);

  // Generate Signing key
  signingKey = rawurlencode(consumerSecret) + "&" + rawurlencode(oauthTokenSecret);

  // Generate oauth_signature
  oauthSignature = base64_encode(hmac_sha1(signingKey, signatureBaseString));

  // Generate Request_Token URL
  oauthHeader = "Authorization: OAuth oauth_consumer_key=\"" + rawurlencode(consumerKey)
    + "\",oauth_nonce=\"" + rawurlencode(oauth_nonce) 
    + "\",oauth_signature_method=\"" + rawurlencode("HMAC-SHA1")
    + "\",oauth_timestamp=\"" + rawurlencode(intToString(t))
    + "\",oauth_version=\"" + rawurlencode("1.0")
    + "\",oauth_token=\"" + rawurlencode(oauthToken)
    + "\",oauth_signature=\"" + rawurlencode(oauthSignature);
  oauthHeader.erase(oauthHeader.size() - 2);
  oauthHeader += "\"";

  // cURL Init
  CURL *curl;
  CURLcode ret;
  struct curl_slist *headers = NULL;
  std::string chunk;

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();

  if (curl == NULL) {
    std::cerr << "curl_easy_init() failed" << std::endl;
    return 1;
  }

  headers = curl_slist_append(headers, "Expect:");
  headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
  headers = curl_slist_append(headers, oauthHeader.c_str());

  curl_easy_setopt(curl, CURLOPT_VERBOSE, curlVerbose);
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_POST, 1);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER , headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, param.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (int)param.length());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (std::string*)&chunk);

  ret = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);

  if(ret != CURLE_OK) {
    std::cerr << "curl_easy_perform() failed." << std::endl;
    return 1;
  }
  if(chunk.find("error")) {
    std::cerr << chunk << std::endl;
    return 1;
  }

  result = chunk;

  return 0;
}

std::string twitpp::update(const std::string& text) {
  if(oauthPostRequest("https://api.twitter.com/1.1/statuses/update.json", "status=" + rawurlencode(text))) {
    return "curl_error";
  }
  return result;
}
