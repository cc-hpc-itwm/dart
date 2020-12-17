#pragma once

#include "net/io_pool.hpp"

#include "http/client.hpp"
#include "http/message.hpp"

#include <boost/filesystem/path.hpp>

namespace http
{
  /**
  * Abstract class that can be used to implement an http server.
  */
  class server
  {
  public:
    /**
    * Constructor.
    *
    * @param pool  the io pool to use
    * @param port  the port number to bind to
    * @param certs folder with certificates
    */
    server(net::io_pool& pool, unsigned short port, const boost::filesystem::path& certs);

    /**
    * Destructor.
    */
    virtual ~server() = default;

    /**
    * Starts the server (non-blocking).
    */
    void run();

    /**
    * Handles incoming messages.
    *
    * @param client  the client that send the message
    * @param message the message itself
    */
    virtual void handle_message(client* client, const message& message) = 0;

    /**
    * Gets the next service from the pool.
    *
    * @param the service
    */
    inline net::io_service& get_service() noexcept { return _pool.get_service(); }

    /**
    * Gets the current context.
    *
    * @param the context
    */
    inline net::context& get_context() noexcept { return _context; }
  private:
    void start_accept();
    void handle_accept(client* client, const boost::system::error_code& error);
  private:
    net::io_pool& _pool;
    net::acceptor _acceptor;
    net::context _context;
  };
}