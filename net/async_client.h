#ifndef TWITPP_NET_ASYNC_CLIENT_H
#define TWITPP_NET_ASYNC_CLIENT_H

#include <string>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/spirit/include/qi.hpp>
#include "response.h"

namespace twitpp {
namespace net {

namespace asio = boost::asio;

class async_client {
public:
  async_client(asio::io_service& io_service, asio::ssl::context& context, const std::string& host, const std::string& path);

  void get(const std::string& header, std::function<void(int&, std::string&)> handler);
  void post(const std::string& header, const std::string& data, std::function<void(int&, std::string&)> handler);

  response response_;

private:
  asio::ip::tcp::resolver resolver_;
  asio::ssl::stream<asio::ip::tcp::socket> socket_;

  std::string host_;
  std::string path_;

  asio::streambuf request_buffer_;
  asio::streambuf response_buffer_;

  std::function<void(int&, std::string&)> handler_;

  void handle_resolve(const boost::system::error_code& err, asio::ip::tcp::resolver::iterator endpoint_iterator);
  void handle_connect(const boost::system::error_code& err);
  void handle_handshake(const boost::system::error_code& err);
  void handle_write(const boost::system::error_code& err);
  void handle_read_status(const boost::system::error_code& err);
  void handle_read_header(const boost::system::error_code& err);
  void handle_read_chunk_size(const boost::system::error_code& err);
  void handle_read_chunk_body(std::size_t content_length, const boost::system::error_code& err);
  void handle_read_content(std::size_t content_length, const boost::system::error_code& err);
  void handle_read_content_all(const boost::system::error_code& err);

  int chunk_parser(const std::string& source, std::size_t& chunk) {
    namespace qi = boost::spirit::qi;
    const qi::rule<std::string::const_iterator, unsigned int()> rule = qi::hex >> qi::lit("\r\n");

    std::string::const_iterator it = source.cbegin();
    qi::parse(it, source.cend(), rule, chunk);

    return std::distance(source.cbegin(), it);
  }
};

}
}

#endif
