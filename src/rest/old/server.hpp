#pragma once

#include "net/io_pool.hpp"

#include "rest/session.hpp"
#include "rest/router.hpp"

#include "http/response.hpp"
#include "http/request.hpp"

namespace rest
{
  class session;

  class server final
  {
  public:
    server(net::io_pool& pool, unsigned short port, router&& router);

    void run();

    http::response process(http::request&& request) const;

    inline net::io_service& get_service() noexcept { return _pool.get_service(); }
    inline net::context& get_context() noexcept { return _context; }
  private:
    void start_accept();
    void handle_accept(session* new_session, const boost::system::error_code& error);
  private:
    net::io_pool& _pool;
    net::acceptor _acceptor;
    net::context _context;
    const router _router;
  };
}