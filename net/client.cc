#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "client.h"

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
                          boost::bind(&async_client::handleResolve, this, asio::placeholders::error, asio::placeholders::iterator));
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
                          boost::bind(&async_client::handleResolve, this, asio::placeholders::error, asio::placeholders::iterator));
}

void async_client::handleResolve(const boost::system::error_code& error, asio::ip::tcp::resolver::iterator endpoint_iterator) {
  if (!error) {
    asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
                        boost::bind(&async_client::handleConnect, this, asio::placeholders::error));
  } else {
    std::cout << "Resolve failed: " << error.value() << "\n";
  }
}

void async_client::handleConnect(const boost::system::error_code& error) {
  if (!error) {
    socket_.async_handshake(asio::ssl::stream_base::client,
                            boost::bind(&async_client::handleHandshake, this, asio::placeholders::error));
  } else {
    std::cout << "Connect failed: " << error.value() << "\n";
  }
}

void async_client::handleHandshake(const boost::system::error_code& error) {
  if (!error) {
    asio::async_write(socket_, request_buffer_, boost::bind(&async_client::handleWrite, this, asio::placeholders::error));
  } else {
    std::cout << "Handshake failed: " << error.value() << "\n";
  }
}

void async_client::handleWrite(const boost::system::error_code& error) {
  if (!error) {
    asio::async_read_until(socket_, response_buffer_, "\r\n",
                           boost::bind(&async_client::handleReadStatus, this, asio::placeholders::error));
  } else {
    std::cout << "Write failed: " << error.value() << "\n";
  }
}

void async_client::handleReadStatus(const boost::system::error_code& error) {
  if (!error) {
    // check response
    std::istream responseStream(&response_buffer_);
    responseStream >> response_.http_version;
    responseStream >> response_.status_code;
    std::getline(responseStream, response_.status_message);

    if (!responseStream || response_.http_version.substr(0, 5) != "HTTP/") {
      std::cout << "Invalid response\n";
      return;
    }

    // read response header
    asio::async_read_until(socket_, response_buffer_, "\r\n\r\n",
                           boost::bind(&async_client::handleReadHeader, this, asio::placeholders::error));
  } else {
    std::cout << "Read status failed: " << error.value() << "\n";
  }
}

void async_client::handleReadHeader(const boost::system::error_code& error) {
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
                             boost::bind(&async_client::handleReadChunkSize, this, asio::placeholders::error));
    } else if (response_.response_header.count("Content-Length") != 0) {
      // use content length
      std::size_t content_length = std::stoi(response_.response_header["Content-Length"]);

      asio::async_read(socket_, response_buffer_,
                       asio::transfer_at_least(content_length - asio::buffer_size(response_buffer_.data())),
                       boost::bind(&async_client::handleReadContent, this, content_length, asio::placeholders::error));
    } else if (response_.response_header.count("content-length") != 0) {
      std::size_t content_length = std::stoi(response_.response_header["content-length"]);

      asio::async_read(socket_, response_buffer_,
                       asio::transfer_at_least(content_length - asio::buffer_size(response_buffer_.data())),
                       boost::bind(&async_client::handleReadContent, this, content_length, asio::placeholders::error));
    } else {
      // other (not working now
      asio::async_read(socket_, response_buffer_, asio::transfer_all(),
                       boost::bind(&async_client::handleReadContentAll, this, asio::placeholders::error));
    }

  } else {
    std::cout << "Read header failed: " << error.value() << "\n";
  }
}

void async_client::handleReadChunkSize(const boost::system::error_code& error) {
  if (!error) {
    if (response_buffer_.size() == 0) {
      return;
    } else if (response_buffer_.size() <= 2) {
      asio::async_read_until(socket_, response_buffer_, "\r\n",
                             boost::bind(&async_client::handleReadChunkSize, this, asio::placeholders::error));
    } else {
      // read chunk size
      std::size_t chunk_size;
      response_buffer_.consume(chunk_parser((std::string)asio::buffer_cast<const char*>(response_buffer_.data()), chunk_size));

      if (chunk_size == 0) {
        // end
        return;
      } else if (chunk_size <= 2) {
        // space only ?
        boost::system::error_code ec;
        asio::read(socket_, response_buffer_,
                   asio::transfer_at_least((chunk_size + 2) - asio::buffer_size(response_buffer_.data())), ec);
        response_buffer_.consume(chunk_size + 2);
        asio::async_read_until(socket_, response_buffer_, "\r\n",
                               boost::bind(&async_client::handleReadChunkSize, this, asio::placeholders::error));
      } else {
        // read chunk
        asio::async_read(socket_, response_buffer_,
                         asio::transfer_at_least(chunk_size - asio::buffer_size(response_buffer_.data())),
                         boost::bind(&async_client::handleReadChunkBody, this, chunk_size, asio::placeholders::error));
      }
    }
  } else if (error != asio::error::eof) {
    std::cout << "Read chunksize failed: " << error.value() << "\n";
  }
}

void async_client::handleReadChunkBody(std::size_t content_length, const boost::system::error_code& error) {
  if (!error) {
    boost::system::error_code ec;
    asio::read(socket_, response_buffer_,
               asio::transfer_at_least((content_length + 2) - asio::buffer_size(response_buffer_.data())), ec);

    response_.response_body.append(boost::asio::buffer_cast<const char*>(response_buffer_.data()), content_length);
    response_buffer_.consume(content_length + 2);
    handler_(response_.status_code, response_.response_body);

    asio::async_read_until(socket_, response_buffer_, "\r\n",
                           boost::bind(&async_client::handleReadChunkSize, this, asio::placeholders::error));
  } else if (error != asio::error::eof) {
    std::cout << "Read chunk failed: " << error.value() << "\n";
  }
}

void async_client::handleReadContent(std::size_t content_length, const boost::system::error_code& error) {
  if (!error) {
    response_.response_body.append(asio::buffers_begin(response_buffer_.data()), asio::buffers_end(response_buffer_.data()));
    response_buffer_.consume(response_buffer_.size());
    handler_(response_.status_code, response_.response_body);
  } else if (error != asio::error::eof) {
    std::cout << "Read content failed: " << error.value() << "\n";
  }
}

void async_client::handleReadContentAll(const boost::system::error_code& error) {
  if (!error) {
    std::ostringstream tmp;
    tmp << &response_buffer_;
    response_.response_body = tmp.str();
    handler_(response_.status_code, response_.response_body);

    // Continue reading remaining data until EOF.
    asio::async_read(socket_, response_buffer_, asio::transfer_at_least(1),
                     boost::bind(&async_client::handleReadContentAll, this, asio::placeholders::error));
  } else if (error != asio::error::eof) {
    std::cout << "Read content all failed: " << error.value() << "\n";
  }
}
}
}
