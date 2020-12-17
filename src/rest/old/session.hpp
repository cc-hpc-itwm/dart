#pragma once

#include "net/types.hpp"
#include "rest/server.hpp"
#include "http/request_builder.hpp"

namespace rest
{
  class server;

  class session final
  {
  public:
    session(server* parent);

    net::ssl_socket::lowest_layer_type& socket();

    void start();
  private:
    void handle_handshake(const boost::system::error_code& error);

    void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred);

    void handle_write(const boost::system::error_code& error);
  private:
    server* _server;
    net::ssl_socket _socket;
    http::request_builder _request_builder;
    enum { max_length = 1024 };
    char _data[max_length];
  };
}