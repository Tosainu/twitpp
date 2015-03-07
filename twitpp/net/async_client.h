#ifndef TWITPP_NET_ASYNC_CLIENT_H
#define TWITPP_NET_ASYNC_CLIENT_H

#include <string>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "method.h"
#include "response.h"

namespace twitpp {
namespace net {

class async_client {
public:
  async_client(const net::method& method, const std::string& url, const std::string& header, const std::string& data);

  void run(const response_handler& handler);

private:
  std::shared_ptr<boost::asio::io_service> io_service_;
  std::shared_ptr<boost::asio::ip::tcp::resolver::query> query_;

  boost::asio::streambuf request_;
  boost::asio::streambuf response_buffer_;

  std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_;

  std::shared_ptr<net::response> response_;
  response_handler handler_;

  void handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
  void handle_connect(const boost::system::error_code& err);
  void handle_handshake(const boost::system::error_code& err);
  void handle_write(const boost::system::error_code& err);
  void handle_read_status(const boost::system::error_code& err);
  void handle_read_header(const boost::system::error_code& err);
  void handle_read_chunk_size(const boost::system::error_code& err);
  void handle_read_chunk_body(std::size_t content_length, const boost::system::error_code& err);
  void handle_read_content(std::size_t content_length, const boost::system::error_code& err);
  void handle_read_content_all(const boost::system::error_code& err);
};

} // namespace net
} // namespace twitpp

#endif
