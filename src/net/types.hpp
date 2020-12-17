#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace net
{
  using ssl_socket = typename boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
  using io_service = boost::asio::io_service;
  using acceptor = boost::asio::ip::tcp::acceptor;
  using context = boost::asio::ssl::context;
  using endpoint = boost::asio::ip::tcp::endpoint;
}