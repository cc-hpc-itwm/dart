#pragma once

#include <string>

#include "net/types.hpp"

#include "rest/server.hpp"

namespace rest
{
  class client final
  {
  public:
    client(net::ssl_socket&& socket);
    ~client() = default;

    response send_request(const std::string& method, const std::string& url, const boost::property_tree::ptree& content);
  private:
    void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred);

    void handle_write(const boost::system::error_code& error);
  private:
    net::ssl_socket _socket;
  };
}