#include "rest/session.hpp"

using namespace rest;

session::session(server* parent)
: _server(parent)
, _socket(parent->get_service(), parent->get_context())
{
}

net::ssl_socket::lowest_layer_type& session::socket()
{
  return _socket.lowest_layer();
}

void session::start()
{
  _socket.async_handshake(boost::asio::ssl::stream_base::server,
    boost::bind(&session::handle_handshake, this,
      boost::asio::placeholders::error));
}

void session::handle_handshake(const boost::system::error_code& error)
{
  if (!error)
  {
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
  }
  else
  {
    delete this;
  }
}

void session::handle_read(const boost::system::error_code& error,
  size_t bytes_transferred)
{
  if (!error)
  {
    _request_builder.add_data(std::vector<char>(_data, _data + bytes_transferred));
    while (auto request = _request_builder.next_request())
    {
      auto response = _server->process(std::move(request.get()));
      boost::asio::async_write(_socket,
        boost::asio::buffer(std::move(response.to_buffer())),
        boost::bind(&session::handle_write, this, 
          boost::asio::placeholders::error));
    }

    _socket.async_read_some(boost::asio::buffer(_data, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
  }
  else
  {
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error)
{
  if (!error)
  {
    /*_socket.async_read_some(boost::asio::buffer(_data, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));*/
  }
  else
  {
    delete this;
  }
}