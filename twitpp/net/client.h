#ifndef TWITPP_NET_CLIENT_H
#define TWITPP_NET_CLIENT_H

#include <map>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "method.h"
#include "response.h"

namespace twitpp {
namespace net {

class client {
public:
  client(const net::method& method, const std::string& url, const std::string& header, const std::string& data);

  void run();

  net::response& response();

private:
  template <typename SocketPtr>
  void read_response(SocketPtr socket);

  std::shared_ptr<boost::asio::io_service> io_service_;
  std::shared_ptr<boost::asio::ip::tcp::resolver::query> query_;

  boost::asio::streambuf request_;

  std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
  std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_ssl_;

  std::shared_ptr<net::response> response_;
};

} // namespace net
} // namespace twitpp

#endif
