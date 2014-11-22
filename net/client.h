#ifndef TWITPP_NET_CLIENT_H
#define TWITPP_NET_CLIENT_H

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "method.h"
#include "response.h"

namespace twitpp {
namespace net {

class client {
public:
  client(const net::method& method, const std::string& url);

  ~client();

  void add_header(const std::string& header);
  void add_content(const std::string& content);

  void run();

  net::response& response();

private:
  void connect_http();
  void connect_https();

  std::shared_ptr<boost::asio::io_service> io_service_;

  boost::asio::ssl::context context_;

  std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
  std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_ssl_;

  boost::asio::ip::tcp::resolver resolver_;

  std::shared_ptr<boost::asio::ip::tcp::resolver::query> query_;

  boost::asio::streambuf request_;
  std::ostream request_stream_;

  bool content_flag_ = false;
  bool is_https_ = false;

  std::shared_ptr<net::response> response_;
};

}
}

#endif
