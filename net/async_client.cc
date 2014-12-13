#include <iostream>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "async_client.h"

namespace twitpp {
namespace net {

namespace asio = boost::asio;

async_client::async_client(asio::io_service& io_service, asio::ssl::context& context, const std::string& host, const std::string& path)
    : resolver_(io_service), socket_(io_service, context), host_(host), path_(path) {}

// GET method
void async_client::get(const std::string& header, std::function<void(int&, std::string&)> handler) {
  handler_ = handler;

  // create request;
  std::ostream request_stream(&request_buffer_);
  request_stream << "GET " << path_ << " HTTP/1.1\r\n";
  request_stream << "Host: " << host_ << "\r\n";
  request_stream << "Accept: */*\r\n";
  if (!header.empty()) {
    request_stream << header << "\r\n";
  }
  request_stream << "Connection: close\r\n\r\n";

  // get server name and host info
  asio::ip::tcp::resolver::query query(host_, "443");
  resolver_.async_resolve(query,
                          boost::bind(&async_client::handle_resolve, this, asio::placeholders::error, asio::placeholders::iterator));
}

// POST method
void async_client::post(const std::string& header, const std::string& data, std::function<void(int&, std::string&)> handler) {
  handler_ = handler;

  // create request;
  std::ostream request_stream(&request_buffer_);
  request_stream << "POST " << path_ << " HTTP/1.1\r\n";
  request_stream << "Host: " << host_ << "\r\n";
  request_stream << "Accept: */*\r\n";
  if (!header.empty()) {
    request_stream << header << "\r\n";
  }
  if (!data.empty()) {
    request_stream << "Content-Length: " << data.length() << "\r\n";
    request_stream << "Content-Type: application/x-www-form-urlencoded\r\n";
    request_stream << "Connection: close\r\n\r\n";
    request_stream << data << "\r\n";
  } else {
    request_stream << "Connection: close\r\n\r\n";
  }

  // get server name and host info
  asio::ip::tcp::resolver::query query(host_, "443");
  resolver_.async_resolve(query,
                          boost::bind(&async_client::handle_resolve, this, asio::placeholders::error, asio::placeholders::iterator));
}

void async_client::handle_resolve(const boost::system::error_code& error, asio::ip::tcp::resolver::iterator endpoint_iterator) {
  if (!error) {
    asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
                        boost::bind(&async_client::handle_connect, this, asio::placeholders::error));
  } else {
    std::cerr << "twitpp::net::async_client: " << "resolve failed: " << error.value() << std::endl;
  }
}

void async_client::handle_connect(const boost::system::error_code& error) {
  if (!error) {
    socket_.async_handshake(asio::ssl::stream_base::client,
                            boost::bind(&async_client::handle_handshake, this, asio::placeholders::error));
  } else {
    std::cerr << "twitpp::net::async_client: " << "connect failed: " << error.value() << std::endl;
  }
}

void async_client::handle_handshake(const boost::system::error_code& error) {
  if (!error) {
    asio::async_write(socket_, request_buffer_, boost::bind(&async_client::handle_write, this, asio::placeholders::error));
  } else {
    std::cerr << "twitpp::net::async_client: " << "handshake failed: " << error.value() << std::endl;
  }
}

void async_client::handle_write(const boost::system::error_code& error) {
  if (!error) {
    asio::async_read_until(socket_, response_buffer_, "\r\n",
                           boost::bind(&async_client::handle_read_status, this, asio::placeholders::error));
  } else {
    std::cerr << "twitpp::net::async_client: " << "write failed: " << error.value() << std::endl;
  }
}

void async_client::handle_read_status(const boost::system::error_code& error) {
  if (!error) {
    // check response
    std::istream responseStream(&response_buffer_);
    responseStream >> response_.http_version;
    responseStream >> response_.status_code;
    std::getline(responseStream, response_.status_message);

    if (!responseStream || response_.http_version.substr(0, 5) != "HTTP/") {
      std::cerr << "twitpp::net::async_client: " << "invalid response" << std::endl;
      return;
    }

    // read response header
    asio::async_read_until(socket_, response_buffer_, "\r\n\r\n",
                           boost::bind(&async_client::handle_read_header, this, asio::placeholders::error));
  } else {
    std::cerr << "twitpp::net::async_client: " << "read status failed: " << error.value() << std::endl;
  }
}

void async_client::handle_read_header(const boost::system::error_code& error) {
  if (!error) {
    // read response header
    std::istream responseStream(&response_buffer_);
    std::string header;
    while (std::getline(responseStream, header) && header != "\r") {
      std::string field_name(header, 0, header.find(":", 0));
      std::string field_body(header, header.find(":", 0) + 2);

      response_.response_header[field_name] = field_body;
    }

    if (response_.response_header.count("transfer-encoding") != 0 ||
        response_.response_header["transfer-encoding"] == "chunked") {
      // chuncked transfer
      asio::async_read_until(socket_, response_buffer_, "\r\n",
                             boost::bind(&async_client::handle_read_chunk_size, this, asio::placeholders::error));
    } else if (response_.response_header.count("Content-Length") != 0) {
      // use content length
      std::size_t content_length = std::stoi(response_.response_header["Content-Length"]);

      asio::async_read(socket_, response_buffer_,
                       asio::transfer_at_least(content_length - asio::buffer_size(response_buffer_.data())),
                       boost::bind(&async_client::handle_read_content, this, content_length, asio::placeholders::error));
    } else if (response_.response_header.count("content-length") != 0) {
      std::size_t content_length = std::stoi(response_.response_header["content-length"]);

      asio::async_read(socket_, response_buffer_,
                       asio::transfer_at_least(content_length - asio::buffer_size(response_buffer_.data())),
                       boost::bind(&async_client::handle_read_content, this, content_length, asio::placeholders::error));
    } else {
      // other (not working now
      asio::async_read(socket_, response_buffer_, asio::transfer_all(),
                       boost::bind(&async_client::handle_read_content_all, this, asio::placeholders::error));
    }

  } else {
    std::cerr << "twitpp::net::async_client: " << "read header failed: " << error.value() << std::endl;
  }
}

void async_client::handle_read_chunk_size(const boost::system::error_code& error) {
  if (!error) {
    if (response_buffer_.size() == 0) {
      return;
    } else if (response_buffer_.size() <= 2) {
      asio::async_read_until(socket_, response_buffer_, "\r\n",
                             boost::bind(&async_client::handle_read_chunk_size, this, asio::placeholders::error));
    } else {
      // read chunk size
      std::size_t chunk_size = std::stoi(static_cast<std::string>(asio::buffer_cast<const char*>(response_buffer_.data())),
                                         nullptr, 16);
      response_buffer_.consume(chunk_size);

      if (chunk_size > 0) {
        // read chunk
        asio::async_read(socket_, response_buffer_,
                         asio::transfer_at_least(chunk_size - asio::buffer_size(response_buffer_.data())),
                         boost::bind(&async_client::handle_read_chunk_body, this, chunk_size, asio::placeholders::error));
      } else {
        // end
        return;
      }
    }
  } else if (error != asio::error::eof) {
    std::cerr << "twitpp::net::async_client: " << "read chunksize failed: " << error.value() << std::endl;
  }
}

void async_client::handle_read_chunk_body(std::size_t content_length, const boost::system::error_code& error) {
  if (!error) {
    boost::system::error_code ec;
    asio::read(socket_, response_buffer_,
               asio::transfer_at_least((content_length + 2) - asio::buffer_size(response_buffer_.data())), ec);

    response_.response_body.append(boost::asio::buffer_cast<const char*>(response_buffer_.data()), content_length);
    response_buffer_.consume(content_length + 2);
    handler_(response_.status_code, response_.response_body);

    asio::async_read_until(socket_, response_buffer_, "\r\n",
                           boost::bind(&async_client::handle_read_chunk_size, this, asio::placeholders::error));
  } else if (error != asio::error::eof) {
    std::cerr << "twitpp::net::async_client: " << "read chunk failed: " << error.value() << std::endl;
  }
}

void async_client::handle_read_content(std::size_t content_length, const boost::system::error_code& error) {
  if (!error) {
    response_.response_body.append(asio::buffers_begin(response_buffer_.data()), asio::buffers_end(response_buffer_.data()));
    response_buffer_.consume(response_buffer_.size());
    handler_(response_.status_code, response_.response_body);
  } else if (error != asio::error::eof) {
    std::cerr << "twitpp::net::async_client: " << "read content failed: " << error.value() << std::endl;
  }
}

void async_client::handle_read_content_all(const boost::system::error_code& error) {
  if (!error) {
    std::ostringstream tmp;
    tmp << &response_buffer_;
    response_.response_body = tmp.str();
    handler_(response_.status_code, response_.response_body);

    // Continue reading remaining data until EOF.
    asio::async_read(socket_, response_buffer_, asio::transfer_at_least(1),
                     boost::bind(&async_client::handle_read_content_all, this, asio::placeholders::error));
  } else if (error != asio::error::eof) {
    std::cerr << "twitpp::net::async_client: " << "read content all failed: " << error.value() << std::endl;
  }
}
}
}
