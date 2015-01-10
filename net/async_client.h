#ifndef TWITPP_NET_ASYNC_CLIENT_H
#define TWITPP_NET_ASYNC_CLIENT_H

#include <string>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "response.h"

namespace twitpp {
namespace net {

class async_client {
public:
  async_client(boost::asio::io_service& io_service, boost::asio::ssl::context& context, const std::string& host, const std::string& path);

  void get(const std::string& header, const response_handler& handler);
  void post(const std::string& header, const std::string& data, const response_handler& handler);

private:
  boost::asio::ip::tcp::resolver resolver_;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;

  std::string host_;
  std::string path_;

  boost::asio::streambuf request_buffer_;
  boost::asio::streambuf response_buffer_;

  response response_;
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

}
}

#endif
