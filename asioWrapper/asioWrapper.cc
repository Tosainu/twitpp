#include <sstream>
#include <string>
#include "asioWrapper.h"

namespace twitpp {
namespace asioWrapper {

  namespace asio = boost::asio;

  // GET method
  Client::Client(asio::io_service& io_service, asio::ssl::context& context,
      const std::string& host, const std::string& path, const std::string& header)
    : io_service_(io_service), resolver_(io_service), socket_(io_service, context) {
      // create request;
      std::ostream request_stream(&request_buffer_);
      request_stream << "GET " << path << " HTTP/1.1\r\n";
      request_stream << "Host: " << host << "\r\n";
      request_stream << "Accept: */*\r\n";
      if(!header.empty()) {
        request_stream << header << "\r\n";
      }
      request_stream << "Connection: close\r\n\r\n";

      // get server name and host info
      asio::ip::tcp::resolver::query query(host, "443");
      resolver_.async_resolve(query, boost::bind(&Client::handleResolve, this,
            asio::placeholders::error, asio::placeholders::iterator));
  }

  // POST method
  Client::Client(asio::io_service& io_service, asio::ssl::context& context,
      const std::string& host, const std::string& path, const std::string& header, const std::string& data)
    : io_service_(io_service), resolver_(io_service), socket_(io_service, context) {
      // create request;
      std::ostream request_stream(&request_buffer_);
      request_stream << "POST " << path << " HTTP/1.1\r\n";
      request_stream << "Host: " << host << "\r\n";
      request_stream << "Accept: */*\r\n";
      if(!header.empty()) {
        request_stream << header << "\r\n";
      }
      if(!data.empty()) {
        request_stream << "Content-Length: " << data.length() << "\r\n";
        request_stream << "Content-Type: application/x-www-form-urlencoded\r\n";
        request_stream << "Connection: close\r\n\r\n";
        request_stream << data << "\r\n";
      }
      else {
        request_stream << "Connection: close\r\n\r\n";
      }

      // get server name and host info
      asio::ip::tcp::resolver::query query(host, "443");
      resolver_.async_resolve(query, boost::bind(&Client::handleResolve, this,
            asio::placeholders::error, asio::placeholders::iterator));
  }

  void Client::handleResolve(const boost::system::error_code& error, asio::ip::tcp::resolver::iterator endpoint_iterator) {
    if(!error) {
      asio::async_connect(socket_.lowest_layer(), endpoint_iterator, boost::bind(&Client::handleConnect, this,
            asio::placeholders::error));
    }
    else {
      std::cout << "Error: " << error.message() << "\n";
    }
  }

  void Client::handleConnect(const boost::system::error_code& error) {
    if(!error) {
      socket_.async_handshake(asio::ssl::stream_base::client, boost::bind(&Client::handleHandshake, this,
            asio::placeholders::error));
    }
    else {
      std::cout << "Connect failed: " << error.message() << "\n";
    }
  }

  void Client::handleHandshake(const boost::system::error_code& error) {
    if(!error) {
      asio::async_write(socket_, request_buffer_, boost::bind(&Client::handleWrite, this,
            asio::placeholders::error));
    }
    else {
      std::cout << "Handshake failed: " << error.message() << "\n";
    }
  }

  void Client::handleWrite(const boost::system::error_code& error) {
    if(!error) {
      asio::async_read_until(socket_, response_buffer_, "\r\n", boost::bind(&Client::handleReadStatus, this,
            asio::placeholders::error));
    }
    else {
      std::cout << "Write failed: " << error.message() << "\n";
    }
  }

  void Client::handleReadStatus(const boost::system::error_code& error) {
    if(!error) {
      // check response
      std::istream responseStream(&response_buffer_);
      responseStream >> response_.http_version;
      responseStream >> response_.status_code;
      std::getline(responseStream, response_.status_message);

      if(!responseStream || response_.http_version.substr(0, 5) != "HTTP/") {
        std::cout << "Invalid response\n";
        return;
      }

      // read response header
      asio::async_read_until(socket_, response_buffer_, "\r\n\r\n", boost::bind(&Client::handleReadHeader, this,
            asio::placeholders::error));
    }
    else {
      std::cout << "Read failed: " << error.message() << "\n";
    }
  }

  void Client::handleReadHeader(const boost::system::error_code& error) {
    if(!error) {
      std::istream responseStream(&response_buffer_);
      std::string header;
      while(std::getline(responseStream, header) && header != "\r") {
        response_.response_header += header;
        response_.response_header += "\n";
      }

      response_.response_header.erase(response_.response_header.end() - 2, response_.response_header.end());

      // read response body
      asio::async_read(socket_, response_buffer_, asio::transfer_at_least(1),
          boost::bind(&Client::handleReadContent, this, asio::placeholders::error));
    }
    else {
      std::cout << "Error: " << error.message() << "\n";
    }
  }

  void Client::handleReadContent(const boost::system::error_code& error) {
    if(!error) {
      std::ostringstream tmp;
      tmp << &response_buffer_;
      response_.response_body += tmp.str();

      // Continue reading remaining data until EOF.
      asio::async_read(socket_, response_buffer_, asio::transfer_at_least(1),
          boost::bind(&Client::handleReadContent, this, asio::placeholders::error));
    }
    else if(error != asio::error::eof) {
      std::cout << "Error: " << error.message() << "\n";
    }
  }

}
}
