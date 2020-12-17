#include "http/client.hpp"

#include "log/log.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

using namespace http;

/**
* Constructor.
*
* @param service         the io_service that this client should use
* @param context         the context that this client should use
* @param delete_on_error if true, the client deletes itself on error
*/
client::client(net::io_service& service, net::context& context, bool delete_on_error)
: _socket(service, context)
, _delete_on_error(delete_on_error)
, _last_error()
{
}

/**
* Getter for the socket.
*
* @return the socket that is used by the client
*/
net::ssl_socket::lowest_layer_type& client::socket()
{
  return _socket.lowest_layer();
}

/**
* Connects the client to a remote endpoint.
*
* @param host the remote host
* @param port the remote port number
*/
void client::connect(const std::string& host, unsigned short port)
{
  _host_and_port = host + ":" + std::to_string(port);
  boost::asio::ip::tcp::resolver resolver(_socket.get_io_service());
  auto endpoints = resolver.resolve(boost::asio::ip::tcp::resolver::query(host, std::to_string(port)));
  boost::asio::connect(socket(), endpoints);
}

/**
* Sends a message.
*
* If the handler != nullptr, sends a message and awaits a response to this
* message. On the other hand, if the handler is zero then this function
* won't start an async_read after the message has been written.
*
* @param message the message to sent
* @param handler an optional handler, if we await a response
*/
void client::send_message(const message& message, const message_handler& handler)
{
  boost::asio::async_write(_socket, boost::asio::buffer(message.to_buffer()),
    boost::bind(&client::handle_write, this, handler, boost::asio::placeholders::error));
}

/**
* Starts read request loop.
*
* Asynchronously reads messages and passes them to the handler.
*
* @param handler the message handler
*/
void client::await_requests(const message_handler& handler)
{
  if (handler == nullptr)
    throw std::runtime_error("[http::client::await_request] handler must not be zero");

  auto endpoint = socket().remote_endpoint();
  _host_and_port = endpoint.address().to_string() + ":" + std::to_string(endpoint.port());

  // First start handshake with client
  _socket.async_handshake(boost::asio::ssl::stream_base::server,
    boost::bind(&client::handle_handshake, this, handler,
      boost::asio::placeholders::error));
}

void client::handle_handshake(const message_handler& handler,
  const boost::system::error_code& error)
{
  if (!error)
  {
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
      boost::bind(&client::handle_read, this, handler,
        boost::asio::placeholders::bytes_transferred,
        boost::asio::placeholders::error));
  }
  else
  {
    log_message::error("[http::client::handle_handshake] client '"
      + _host_and_port
      + "' produced error: "
      + error.message());

    _last_error = error;
    if(_delete_on_error)
      delete this;
  }
}

void client::handle_read(const message_handler& handler,
  size_t bytes_transferred,
  const boost::system::error_code& error)
{
  if(!error)
  {
    log_message::info(std::string(_data, bytes_transferred));
    _message_builder.add_data(std::vector<char>(_data, _data + bytes_transferred));
    auto message = _message_builder.next_message();
    while (message)
    {
      handler(this, message.get());
      message = _message_builder.next_message();
    }

    _socket.async_read_some(boost::asio::buffer(_data, max_length),
      boost::bind(&client::handle_read, this, handler,
        boost::asio::placeholders::bytes_transferred,
        boost::asio::placeholders::error));
  }
  else
  {
    log_message::error("[http::client::handle_read] client '"
      + _host_and_port
      + "' produced error: "
      + error.message());

    _last_error = error;
    if (_delete_on_error)
      delete this;
  }
}

void client::handle_write(const message_handler& handler,
  const boost::system::error_code& error)
{
  if (!error)
  {
    if (handler)
    {
      // If we have a handler, we await a response!
      _socket.async_read_some(boost::asio::buffer(_data, max_length),
        boost::bind(&client::handle_read, this, handler,
          boost::asio::placeholders::bytes_transferred,
          boost::asio::placeholders::error));
    }
  }
  else
  {
    log_message::error("[http::client::handle_write] client '"
      + _host_and_port
      + "' produced error: "
      + error.message());

    _last_error = error;
    if (_delete_on_error)
      delete this;
  }
}
