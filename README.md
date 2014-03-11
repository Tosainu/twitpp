# twitpp - A Twitter Library for C++

## Usage

### Set ConsumerKey
    twitpp hoge("Consumer", "ConsumerSecret");

or

    twitpp fuga("Consumer", "ConsumerSecret", "OauthToken", "OauthTokenSecret");

### Oauth Authentication
    hoge.oauthGetAuthorizeUrl()
    std::cout << hoge.authorizeUrl << std::endl;
    hoge.oauthGetAccessToken("pin");

### Post Status(Tweet)
    hoge.update("Hello World!");

## LICENCE
* twitpp released under the [MIT license](LICENSE).

