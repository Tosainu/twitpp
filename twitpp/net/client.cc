#include <ostream>
#include <stdexcept>
#include <tuple>
#include "../util/util.h"
#include "client.h"

namespace twitpp {
namespace net {

client::client(const net::method& method, const std::string& url, const std::string& header, const std::string& data)
  : io_service_(std::make_shared<boost::asio::io_service>()), response_(std::make_shared<net::response>()) {
  auto parsed_url = util::url_parser(url);

  if (!parsed_url) {
    throw std::invalid_argument("failed to parse the url");
  }

  if (std::get<0>(*parsed_url) == "https") {
    boost::asio::ssl::context context(boost::asio::ssl::context_base::tlsv12);
    context.set_verify_mode(boost::asio::ssl::verify_none);

    socket_ = nullptr;
    socket_ssl_ = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(*io_service_, context);
  } else {
    socket_ = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_);
    socket_ssl_ = nullptr;
  }

  query_ = std::make_shared<boost::asio::ip::tcp::resolver::query>(std::get<1>(*parsed_url), std::get<0>(*parsed_url));

  std::ostream request_stream(&request_buf_);

  switch (method) {
    case net::method::GET:
      request_stream << "GET " << std::get<2>(*parsed_url);

      if (!data.empty()) {
        request_stream << "?" << data;
      }

      request_stream << " HTTP/1.1\r\n";
      break;
    case net::method::POST:
      request_stream << "POST " << std::get<2>(*parsed_url) << std::get<3>(*parsed_url) << " HTTP/1.1\r\n";
      break;
  }

  request_stream << "Host: " << std::get<1>(*parsed_url) << "\r\n";
  request_stream << "Accept: */*\r\n";

  if (!header.empty()) {
    request_stream << header << "\r\n";
  }

  if (method == net::method::POST && !data.empty()) {
    request_stream << "Content-Length: " << data.length() << "\r\n";
    request_stream << "Content-Type: application/x-www-form-urlencoded\r\n";
    request_stream << "Connection: close\r\n\r\n";
    request_stream << data << "\r\n";
  } else {
    request_stream << "Connection: close\r\n\r\n";
  }
}

void client::run() {
  // name resolving
  boost::asio::ip::tcp::resolver resolver(*io_service_);
  auto endpoint_iterator = resolver.resolve(*query_);

  // creaate connection
  if (socket_ssl_) {
    socket_ssl_->lowest_layer().connect(*endpoint_iterator);
    socket_ssl_->handshake(boost::asio::ssl::stream_base::client);

    read_response(socket_ssl_);
  } else {
    boost::asio::connect(*socket_, endpoint_iterator);

    read_response(socket_);
  }
}

template <typename SocketPtr>
void client::read_response(SocketPtr socket) {
  // send requests
  boost::asio::write(*socket, request_buf_);

  // read status
  boost::asio::streambuf response_buf;
  boost::asio::read_until(*socket, response_buf, "\r\n");

  std::istream response_stream(&response_buf);
  response_stream >> response_->http_version >> response_->status_code >> std::ws;
  std::getline(response_stream, response_->status_message);

  if(!response_stream || response_->http_version.substr(0, 5) != "HTTP/") {
    throw std::runtime_error("invalid response");
  }

  // read header
  boost::asio::read_until(*socket, response_buf, "\r\n\r\n");

  for (std::string s; std::getline(response_stream, s, ':') && s[0] != '\r';) {
    response_stream >> std::ws;
    std::getline(response_stream, response_->header[s], '\r');
    response_stream >> std::ws;
  }

  // read until EOF
  boost::system::error_code error;
  while(boost::asio::read(*socket, response_buf, boost::asio::transfer_all(), error));
  response_->body.assign(boost::asio::buffers_begin(response_buf.data()), boost::asio::buffers_end(response_buf.data()));
  response_buf.consume(response_buf.size());

  if(error != boost::asio::error::eof) {
    throw boost::system::system_error(error);
  }
}

net::response& client::response() {
  return *response_;
}

} // namespace net
} // namespace twitpp
