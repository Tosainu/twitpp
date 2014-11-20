#ifndef TWITPP_CLIENT_H
#define TWITPP_CLIENT_H

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

  void handleResolve(const boost::system::error_code& err, asio::ip::tcp::resolver::iterator endpoint_iterator);
  void handleConnect(const boost::system::error_code& err);
  void handleHandshake(const boost::system::error_code& err);
  void handleWrite(const boost::system::error_code& err);
  void handleReadStatus(const boost::system::error_code& err);
  void handleReadHeader(const boost::system::error_code& err);
  void handleReadChunkSize(const boost::system::error_code& err);
  void handleReadChunkBody(std::size_t content_length, const boost::system::error_code& err);
  void handleReadContent(std::size_t content_length, const boost::system::error_code& err);
  void handleReadContentAll(const boost::system::error_code& err);

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
