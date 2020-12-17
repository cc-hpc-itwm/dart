#pragma once

#include <functional>
#include <string>

#include "net/types.hpp"
#include "http/message.hpp"
#include "http/message_builder.hpp"

namespace http
{
  /**
  * A class for sending and receiving http messages.
  */
  class client final
  {
  public:
    using message_handler = std::function<void(client*, const message&)>;
  public:
    /**
    * Constructor.
    *
    * @param service         the io_service that this client should use
    * @param context         the context that this client should use
    * @param delete_on_error if true, the client deletes itself on error
    */
    client(net::io_service& service, net::context& context, bool delete_on_error = false);

    /**
    * Destructor.
    */
    ~client() = default;

    /**
    * Getter for the socket.
    *
    * @return the socket that is used by the client
    */
    net::ssl_socket::lowest_layer_type& socket();

    /**
    * Connects the client to a remote endpoint.
    *
    * @param host the remote host
    * @param port the remote port number
    */
    void connect(const std::string& host, unsigned short port);

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
    void send_message(const message& message, const message_handler& handler = nullptr);

    /**
    * Starts read request loop.
    *
    * Asynchronously reads messages and passes them to the handler.
    *
    * @param handler the message handler
    */
    void await_requests(const message_handler& handler);

    /**
    * Returns the last error that has occured.
    *
    * @return the last error
    */
    inline auto last_error() const noexcept { return _last_error; }

    /**
    * Gets the host and port string.
    *
    * @return the host and port string
    */
    inline const auto& host_and_port() const noexcept { return _host_and_port; }
  private:
    void handle_handshake(const message_handler& handler,
      const boost::system::error_code& error);

    void handle_read(const message_handler& handler,
      size_t bytes_transferred,
      const boost::system::error_code& error);

    void handle_write(const message_handler& handler,
      const boost::system::error_code& error);
  private:
    net::ssl_socket _socket;
    bool _delete_on_error;
    http::message_builder _message_builder;
    boost::system::error_code _last_error;
    std::string _host_and_port;
    enum { max_length = 1024 };
    char _data[max_length];
  };
}