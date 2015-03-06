#include <iostream>
#include <boost/bind.hpp>
#include "../util/util.h"
#include "async_client.h"

namespace twitpp {
namespace net {

namespace asio = boost::asio;

async_client::async_client(const net::method& method, const std::string& url, const std::string& header, const std::string& data)
    : io_service_(std::make_shared<boost::asio::io_service>()), context_(boost::asio::ssl::context_base::tlsv12),
      resolver_(*io_service_), request_stream_(&request_), response_(std::make_shared<net::response>()) {
  auto parsed_url = util::url_parser(url);

  if (!parsed_url) {
    throw std::invalid_argument("failed to parse the url");
  }

  context_.set_verify_mode(boost::asio::ssl::verify_none);
  socket_ = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(*io_service_, context_);

  query_ = std::make_shared<boost::asio::ip::tcp::resolver::query>(std::get<1>(*parsed_url), std::get<0>(*parsed_url));

  switch (method) {
    case net::method::GET:
      request_stream_ << "GET " << std::get<2>(*parsed_url);

      if (!data.empty()) {
        request_stream_ << "?" << data;
      }

      request_stream_ << " HTTP/1.1\r\n";
      break;
    case net::method::POST:
      request_stream_ << "POST " << std::get<2>(*parsed_url) << std::get<3>(*parsed_url) << " HTTP/1.1\r\n";
      break;
  }

  request_stream_ << "Host: " << std::get<1>(*parsed_url) << "\r\n";
  request_stream_ << "Accept: */*\r\n";

  if (!header.empty()) {
    request_stream_ << header << "\r\n";
  }

  if (method == net::method::POST && !data.empty()) {
    request_stream_ << "Content-Length: " << data.length() << "\r\n";
    request_stream_ << "Content-Type: application/x-www-form-urlencoded\r\n";
    request_stream_ << "Connection: close\r\n\r\n";
    request_stream_ << data << "\r\n";
  } else {
    request_stream_ << "Connection: close\r\n\r\n";
  }
}

void async_client::run(const response_handler& handler) {
  handler_ = handler;

  // name resolving
  resolver_.async_resolve(
      *query_, boost::bind(&async_client::handle_resolve, this, asio::placeholders::error, asio::placeholders::iterator));

  io_service_->run();
}

void async_client::handle_resolve(const boost::system::error_code& error, asio::ip::tcp::resolver::iterator endpoint_iterator) {
  if (!error) {
    asio::async_connect(socket_->lowest_layer(), endpoint_iterator,
                        boost::bind(&async_client::handle_connect, this, asio::placeholders::error));
  } else {
    std::cerr << "twitpp::net::async_client: " << "resolve failed: " << error.value() << std::endl;
  }
}

void async_client::handle_connect(const boost::system::error_code& error) {
  if (!error) {
    socket_->async_handshake(asio::ssl::stream_base::client,
                            boost::bind(&async_client::handle_handshake, this, asio::placeholders::error));
  } else {
    std::cerr << "twitpp::net::async_client: " << "connect failed: " << error.value() << std::endl;
  }
}

void async_client::handle_handshake(const boost::system::error_code& error) {
  if (!error) {
    asio::async_write(*socket_, request_, boost::bind(&async_client::handle_write, this, asio::placeholders::error));
  } else {
    std::cerr << "twitpp::net::async_client: " << "handshake failed: " << error.value() << std::endl;
  }
}

void async_client::handle_write(const boost::system::error_code& error) {
  if (!error) {
    asio::async_read_until(*socket_, response_buffer_, "\r\n",
                           boost::bind(&async_client::handle_read_status, this, asio::placeholders::error));
  } else {
    std::cerr << "twitpp::net::async_client: " << "write failed: " << error.value() << std::endl;
  }
}

void async_client::handle_read_status(const boost::system::error_code& error) {
  if (!error) {
    // check response
    std::istream response_stream(&response_buffer_);
    response_stream >> response_->http_version >> response_->status_code >> std::ws;
    std::getline(response_stream, response_->status_message);

    if (!response_stream || response_->http_version.substr(0, 5) != "HTTP/") {
      std::cerr << "twitpp::net::async_client: " << "invalid response" << std::endl;
      return;
    }

    // read response header
    asio::async_read_until(*socket_, response_buffer_, "\r\n\r\n",
                           boost::bind(&async_client::handle_read_header, this, asio::placeholders::error));
  } else {
    std::cerr << "twitpp::net::async_client: " << "read status failed: " << error.value() << std::endl;
  }
}

void async_client::handle_read_header(const boost::system::error_code& error) {
  if (!error) {
    // read response header
    std::istream response_stream(&response_buffer_);
    for (std::string s; std::getline(response_stream, s, ':') && s[0] != '\r';) {
      response_stream >> std::ws;
      std::getline(response_stream, response_->header[s], '\r');
      response_stream >> std::ws;
    }

    if (response_->header["transfer-encoding"] == "chunked") {
      // chuncked transfer
      asio::async_read_until(*socket_, response_buffer_, "\r\n",
                             boost::bind(&async_client::handle_read_chunk_size, this, asio::placeholders::error));
    } else if (response_->header.count("Content-Length") != 0) {
      // use content length
      std::size_t content_length = std::stoi(response_->header["Content-Length"]);

      asio::async_read(*socket_, response_buffer_,
                       asio::transfer_at_least(content_length - asio::buffer_size(response_buffer_.data())),
                       boost::bind(&async_client::handle_read_content, this, content_length, asio::placeholders::error));
    } else if (response_->header.count("content-length") != 0) {
      std::size_t content_length = std::stoi(response_->header["content-length"]);

      asio::async_read(*socket_, response_buffer_,
                       asio::transfer_at_least(content_length - asio::buffer_size(response_buffer_.data())),
                       boost::bind(&async_client::handle_read_content, this, content_length, asio::placeholders::error));
    } else {
      // other (not working now
      asio::async_read(*socket_, response_buffer_, asio::transfer_all(),
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
      response_buffer_.consume(response_buffer_.size());
      asio::async_read_until(*socket_, response_buffer_, "\r\n",
                             boost::bind(&async_client::handle_read_chunk_size, this, asio::placeholders::error));
    } else {
      // read chunk size
      std::size_t chunk_size = std::strtoul(asio::buffer_cast<const char*>(response_buffer_.data()), nullptr, 16);
      response_buffer_.consume(chunk_size);

      if (chunk_size > 0) {
        // read chunk
        asio::async_read(*socket_, response_buffer_,
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
    asio::read(*socket_, response_buffer_,
               asio::transfer_at_least((content_length + 2) - asio::buffer_size(response_buffer_.data())), ec);

    response_->body.append(boost::asio::buffer_cast<const char*>(response_buffer_.data()), content_length);
    response_buffer_.consume(content_length + 2);
    handler_(*response_);

    asio::async_read_until(*socket_, response_buffer_, "\r\n",
                           boost::bind(&async_client::handle_read_chunk_size, this, asio::placeholders::error));
  } else if (error != asio::error::eof) {
    std::cerr << "twitpp::net::async_client: " << "read chunk failed: " << error.value() << std::endl;
  }
}

void async_client::handle_read_content(std::size_t content_length, const boost::system::error_code& error) {
  if (!error) {
    response_->body.append(asio::buffers_begin(response_buffer_.data()), asio::buffers_end(response_buffer_.data()));
    response_buffer_.consume(content_length);
    handler_(*response_);
  } else if (error != asio::error::eof) {
    std::cerr << "twitpp::net::async_client: " << "read content failed: " << error.value() << std::endl;
  }
}

void async_client::handle_read_content_all(const boost::system::error_code& error) {
  if (!error) {
    std::ostringstream tmp;
    tmp << &response_buffer_;
    response_->body = tmp.str();
    handler_(*response_);

    // Continue reading remaining data until EOF.
    asio::async_read(*socket_, response_buffer_, asio::transfer_at_least(1),
                     boost::bind(&async_client::handle_read_content_all, this, asio::placeholders::error));
  } else if (error != asio::error::eof) {
    std::cerr << "twitpp::net::async_client: " << "read content all failed: " << error.value() << std::endl;
  }
}

} // namespace net
} // namespace twitpp
