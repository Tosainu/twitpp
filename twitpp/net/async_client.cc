#include <iostream>
#include <boost/bind.hpp>
#include "../util/util.h"
#include "async_client.h"

namespace twitpp {
namespace net {

namespace asio = boost::asio;

async_client::async_client(const net::method& method, const std::string& url, const std::string& header, const std::string& data)
    : io_service_(std::make_shared<asio::io_service>()), response_(std::make_shared<net::response>()) {
  auto parsed_url = util::url_parser(url);

  if (!parsed_url) {
    std::cerr << "error: failed to parse the url" << std::endl;
    return;
  }

  asio::ssl::context context(asio::ssl::context_base::tlsv12);
  context.set_verify_mode(asio::ssl::verify_none);
  socket_ = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(*io_service_, context);

  query_ = std::make_shared<asio::ip::tcp::resolver::query>(std::get<1>(*parsed_url), std::get<0>(*parsed_url));

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

void async_client::run(const response_handler& handler) {
  handler_ = handler;

  // name resolving
  asio::ip::tcp::resolver resolver(*io_service_);
  resolver.async_resolve(
      *query_, boost::bind(&async_client::handle_resolve, this, asio::placeholders::error, asio::placeholders::iterator));

  io_service_->run();
}

void async_client::handle_resolve(const boost::system::error_code& error, asio::ip::tcp::resolver::iterator endpoint_iterator) {
  if (error) {
    std::cerr << "failed to resolve: " << error.message() << std::endl;
    return;
  }

  asio::async_connect(socket_->lowest_layer(), endpoint_iterator,
                      boost::bind(&async_client::handle_connect, this, asio::placeholders::error));
}

void async_client::handle_connect(const boost::system::error_code& error) {
  if (error) {
    std::cerr << "failed to connect : " << error.message() << std::endl;
    return;
  }

  socket_->async_handshake(asio::ssl::stream_base::client,
                           boost::bind(&async_client::handle_handshake, this, asio::placeholders::error));
}

void async_client::handle_handshake(const boost::system::error_code& error) {
  if (error) {
    std::cerr << "failed to handshake: " << error.message() << std::endl;
    return;
  }

  asio::async_write(*socket_, request_buf_, boost::bind(&async_client::handle_write, this, asio::placeholders::error));
}

void async_client::handle_write(const boost::system::error_code& error) {
  if (error) {
    std::cerr << "failed to write: " << error.message() << std::endl;
    return;
  }

  asio::async_read_until(*socket_, response_buf_, "\r\n",
                         boost::bind(&async_client::handle_read_status, this, asio::placeholders::error));
}

void async_client::handle_read_status(const boost::system::error_code& error) {
  if (error) {
    std::cerr << "failed to read status: " << error.message() << std::endl;
    return;
  }

  std::istream response_stream(&response_buf_);
  response_stream >> response_->http_version >> response_->status_code >> std::ws;
  std::getline(response_stream, response_->status_message);

  // check response
  if (!response_stream || response_->http_version.substr(0, 5) != "HTTP/") {
    std::cerr << "error: invalid response" << std::endl;
    return;
  }

  asio::async_read_until(*socket_, response_buf_, "\r\n\r\n",
                         boost::bind(&async_client::handle_read_header, this, asio::placeholders::error));
}

void async_client::handle_read_header(const boost::system::error_code& error) {
  if (error) {
    std::cerr << "failed to read header: " << error.message() << std::endl;
    return;
  }

  // read response header
  std::istream response_stream(&response_buf_);
  for (std::string s; std::getline(response_stream, s, ':') && s[0] != '\r';) {
    response_stream >> std::ws;
    std::getline(response_stream, response_->header[s], '\r');
    response_stream >> std::ws;
  }

  if (response_->header["transfer-encoding"] == "chunked") {
    // chuncked transfer
    asio::async_read_until(*socket_, response_buf_, "\r\n",
                           boost::bind(&async_client::handle_read_chunk_size, this, asio::placeholders::error));
  } else if (response_->header.count("Content-Length") != 0) {
    // use content length
    std::size_t content_length = std::stoi(response_->header["Content-Length"]);

    asio::async_read(*socket_, response_buf_,
                     asio::transfer_at_least(content_length - asio::buffer_size(response_buf_.data())),
                     boost::bind(&async_client::handle_read_content, this, content_length, asio::placeholders::error));
  } else if (response_->header.count("content-length") != 0) {
    std::size_t content_length = std::stoi(response_->header["content-length"]);

    asio::async_read(*socket_, response_buf_,
                     asio::transfer_at_least(content_length - asio::buffer_size(response_buf_.data())),
                     boost::bind(&async_client::handle_read_content, this, content_length, asio::placeholders::error));
  } else {
    // other (not working now
    asio::async_read(*socket_, response_buf_, asio::transfer_all(),
                     boost::bind(&async_client::handle_read_content_all, this, asio::placeholders::error));
  }
}

void async_client::handle_read_chunk_size(const boost::system::error_code& error) {
  if (!error) {
    if (response_buf_.size() == 0) {
      return;
    } else if (response_buf_.size() <= 2) {
      response_buf_.consume(response_buf_.size());
      asio::async_read_until(*socket_, response_buf_, "\r\n",
                             boost::bind(&async_client::handle_read_chunk_size, this, asio::placeholders::error));
    } else {
      // read chunk size
      std::size_t chunk_size = std::strtoul(asio::buffer_cast<const char*>(response_buf_.data()), nullptr, 16);
      response_buf_.consume(chunk_size);

      if (chunk_size > 0) {
        // read chunk
        asio::async_read(*socket_, response_buf_,
                         asio::transfer_at_least(chunk_size - asio::buffer_size(response_buf_.data())),
                         boost::bind(&async_client::handle_read_chunk_body, this, chunk_size, asio::placeholders::error));
      } else {
        // end
        return;
      }
    }
  } else if (error != asio::error::eof) {
    std::cerr << "failed to read chunksize: " << error.message() << std::endl;
  }
}

void async_client::handle_read_chunk_body(std::size_t content_length, const boost::system::error_code& error) {
  if (!error) {
    boost::system::error_code ec;
    asio::read(*socket_, response_buf_,
               asio::transfer_at_least((content_length + 2) - asio::buffer_size(response_buf_.data())), ec);

    response_->body.append(asio::buffer_cast<const char*>(response_buf_.data()), content_length);
    response_buf_.consume(content_length + 2);
    handler_(*response_);

    asio::async_read_until(*socket_, response_buf_, "\r\n",
                           boost::bind(&async_client::handle_read_chunk_size, this, asio::placeholders::error));
  } else if (error != asio::error::eof) {
    std::cerr << "failed to read chunk: " << error.message() << std::endl;
  }
}

void async_client::handle_read_content(std::size_t content_length, const boost::system::error_code& error) {
  if (!error) {
    response_->body.append(asio::buffers_begin(response_buf_.data()), asio::buffers_end(response_buf_.data()));
    response_buf_.consume(content_length);
    handler_(*response_);
  } else if (error != asio::error::eof) {
    std::cerr << "failed to read content: " << error.message() << std::endl;
  }
}

void async_client::handle_read_content_all(const boost::system::error_code& error) {
  if (!error) {
    std::ostringstream tmp;
    tmp << &response_buf_;
    response_->body = tmp.str();
    handler_(*response_);

    // Continue reading remaining data until EOF.
    asio::async_read(*socket_, response_buf_, asio::transfer_at_least(1),
                     boost::bind(&async_client::handle_read_content_all, this, asio::placeholders::error));
  } else if (error != asio::error::eof) {
    std::cerr << "failed to read content: " << error.message() << std::endl;
  }
}

} // namespace net
} // namespace twitpp
