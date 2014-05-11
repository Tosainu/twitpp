#ifndef TWITPP_ASIO_WRAPPER_H
#define TWITPP_ASIO_WRAPPER_H

#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>

namespace twitpp {
namespace asioWrapper {

  namespace asio = boost::asio;

  struct Response {
    std::string http_version;
    unsigned int status_code;
    std::string status_message;
    std::string response_header;
    std::string response_body;
  };

  class Client {
    public:
      Client(asio::io_service& io_service, asio::ssl::context& context,
          const std::string& host, const std::string& path, const std::string& header);
      Client(asio::io_service& io_service, asio::ssl::context& context,
          const std::string& host, const std::string& path, const std::string& header, const std::string& data);

      Response response_;

    private:
      asio::io_service& io_service_;
      asio::ip::tcp::resolver resolver_;
      asio::ssl::stream<asio::ip::tcp::socket> socket_;

      asio::streambuf request_buffer_;
      asio::streambuf response_buffer_;

      void handleResolve(const boost::system::error_code& err, asio::ip::tcp::resolver::iterator endpoint_iterator);
      void handleConnect(const boost::system::error_code& err);
      void handleHandshake(const boost::system::error_code& err);
      void handleWrite(const boost::system::error_code& err);
      void handleReadStatus(const boost::system::error_code& err);
      void handleReadHeader(const boost::system::error_code& err);
      void handleReadContent(const boost::system::error_code& err);
  };

}
}

#endif
