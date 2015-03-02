#include <stdexcept>
#include <tuple>
#include "../util/util.h"
#include "client.h"

namespace twitpp {
namespace net {

client::client(const net::method& method, const std::string& url)
  : io_service_(std::make_shared<boost::asio::io_service>()), context_(boost::asio::ssl::context_base::tlsv12),
    resolver_(*io_service_), request_stream_(&request_), response_(std::make_shared<net::response>()) {
  auto parsed_url = util::url_parser(url);

  if (!parsed_url) {
    throw std::invalid_argument("failed to parse the url");
  }

  if (std::get<0>(*parsed_url) == "https") {
    context_.set_verify_mode(boost::asio::ssl::verify_none);

    socket_ = nullptr;
    socket_ssl_ = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(*io_service_, context_);
  } else {
    socket_ = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_);
    socket_ssl_ = nullptr;
  }

  query_ = std::make_shared<boost::asio::ip::tcp::resolver::query>(std::get<1>(*parsed_url), std::get<0>(*parsed_url));

  switch (method) {
    case net::method::GET:
      request_stream_ << "GET " << std::get<2>(*parsed_url) << std::get<3>(*parsed_url) << " HTTP/1.1\r\n";
      break;
    case net::method::POST:
      request_stream_ << "POST " << std::get<2>(*parsed_url) << std::get<3>(*parsed_url) << " HTTP/1.1\r\n";
      break;
  }

  request_stream_ << "Host: " << std::get<1>(*parsed_url) << "\r\n";
  request_stream_ << "Accept: */*\r\n";
}

void client::add_header(const std::string& header) {
  request_stream_ << header << "\r\n";
}

void client::add_content(const std::string& content) {
  if (!content_flag_) {
    request_stream_ << "Content-Length: " << content.length() << "\r\n";
    request_stream_ << "Content-Type: application/x-www-form-urlencoded\r\n";
    request_stream_ << "Connection: close\r\n\r\n";
    request_stream_ << content << "\r\n";
    content_flag_ = true;
  }
}

void client::run() {
  if (!content_flag_) {
    request_stream_ << "Connection: close\r\n\r\n" << std::flush;
  }

  // name resolving
  boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver_.resolve(*query_);

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

template <typename socket_ptr>
void client::read_response(socket_ptr socket) {
  // send requests
  boost::asio::write(*socket, request_);

  // read status
  boost::asio::streambuf response;
  boost::asio::read_until(*socket, response, "\r\n");

  std::istream response_stream(&response);
  response_stream >> response_->http_version >> response_->status_code >> std::ws;
  std::getline(response_stream, response_->status_message);

  if(!response_stream || response_->http_version.substr(0, 5) != "HTTP/") {
    throw std::runtime_error("invalid response");
  }

  // read header
  boost::asio::read_until(*socket, response, "\r\n\r\n");

  for (std::string s; std::getline(response_stream, s, ':') && s[0] != '\r';) {
    response_stream >> std::ws;
    std::getline(response_stream, response_->header[s], '\r');
    response_stream >> std::ws;
  }

  // read until EOF
  boost::system::error_code error;
  while(boost::asio::read(*socket, response, boost::asio::transfer_all(), error));
  response_->body.assign(boost::asio::buffers_begin(response.data()), boost::asio::buffers_end(response.data()));
  response.consume(response.size());

  if(error != boost::asio::error::eof) {
    throw boost::system::system_error(error);
  }
}

net::response& client::response() {
  return *response_;
}

} // namespace net
} // namespace twitpp
