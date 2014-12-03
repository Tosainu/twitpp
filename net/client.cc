#include <stdexcept>
#include <tuple>
#include <boost/xpressive/xpressive.hpp>
#include "../util/util.h"
#include "client.h"

namespace twitpp {
namespace net {

client::client(const net::method& method, const std::string& url)
  : io_service_(std::make_shared<boost::asio::io_service>()), context_(boost::asio::ssl::context_base::tlsv12),
    resolver_(*io_service_), request_stream_(&request_), response_(std::make_shared<net::response>()) {
  auto parsed_url = util::url_parser(url);

  if (std::get<0>(parsed_url) == "https") {
    is_https_ = true;
    context_.set_verify_mode(boost::asio::ssl::verify_none);

    socket_ = nullptr;
    socket_ssl_ = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(*io_service_, context_);
  } else {
    socket_ = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_);
    socket_ssl_ = nullptr;
  }

  query_ = std::make_shared<boost::asio::ip::tcp::resolver::query>(std::get<1>(parsed_url), std::get<0>(parsed_url));

  switch (method) {
    case net::method::GET:
      request_stream_ << "GET " << std::get<2>(parsed_url) << " HTTP/1.1\r\n";
      break;
    case net::method::POST:
      request_stream_ << "POST " << std::get<2>(parsed_url) << " HTTP/1.1\r\n";
      break;
  }

  request_stream_ << "Host: " << std::get<1>(parsed_url) << "\r\n";
  request_stream_ << "Accept: */*\r\n";
}

client::~client(){}

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
  } else {
    // throw 
  }
}

void client::run() {
  if (!content_flag_) {
    request_stream_ << "Connection: close\r\n\r\n" << std::flush;
  }

  if (is_https_) {
    connect_https();
  } else {
    connect_http();
  }
}

void client::connect_http() {
  // get a list of endpoints
  boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver_.resolve(*query_);

  // creaate connection
  boost::asio::connect(*socket_, endpoint_iterator);

  // send requests
  boost::asio::write(*socket_, request_);

  // read response
  boost::asio::streambuf response;
  boost::asio::read_until(*socket_, response, "\r\n");

  // check responce
  std::istream response_stream(&response);

  response_stream >> response_->http_version;
  response_stream >> response_->status_code;
  std::getline(response_stream, response_->status_message);

  if(!response_stream || response_->http_version.substr(0, 5) != "HTTP/") {
    throw std::runtime_error("invalid response");
  }

  // read response header
  boost::asio::read_until(*socket_, response, "\r\n\r\n");
  std::string header;

  while (std::getline(response_stream, header) && header != "\r") {
    std::string field_name(header, 0, header.find(":", 0));
    std::string field_body(header, header.find(":", 0) + 2);

    response_->response_header[field_name] = field_body;
  }

  // read until EOF
  boost::system::error_code error;
  while(boost::asio::read(*socket_, response, boost::asio::transfer_all(), error));
  response_->response_body.assign(boost::asio::buffers_begin(response.data()), boost::asio::buffers_end(response.data()));
  response.consume(response.size());

  if(error != boost::asio::error::eof) {
    throw boost::system::system_error(error);
  }
}

void client::connect_https() {
  // get a list of endpoints
  boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver_.resolve(*query_);

  // creaate connection
  socket_ssl_->lowest_layer().connect(*endpoint_iterator);
  socket_ssl_->handshake(boost::asio::ssl::stream_base::client);

  // send requests
  boost::asio::write(*socket_ssl_, request_);

  // read response
  boost::asio::streambuf response;
  boost::asio::read_until(*socket_ssl_, response, "\r\n");

  // check responce
  std::istream response_stream(&response);

  response_stream >> response_->http_version;
  response_stream >> response_->status_code;
  std::getline(response_stream, response_->status_message);

  if(!response_stream || response_->http_version.substr(0, 5) != "HTTP/") {
    throw std::runtime_error("invalid response");
  }

  // read response header
  boost::asio::read_until(*socket_ssl_, response, "\r\n\r\n");
  std::string header;

  while (std::getline(response_stream, header) && header != "\r") {
    std::string field_name(header, 0, header.find(":", 0));
    std::string field_body(header, header.find(":", 0) + 2);

    response_->response_header[field_name] = field_body;
  }

  // read until EOF
  boost::system::error_code error;
  while(boost::asio::read(*socket_ssl_, response, boost::asio::transfer_all(), error));
  response_->response_body.assign(boost::asio::buffers_begin(response.data()), boost::asio::buffers_end(response.data()));
  response.consume(response.size());

  if(error != boost::asio::error::eof) {
    throw boost::system::system_error(error);
  }
}

net::response& client::response() {
  return *response_;
}

}
}
