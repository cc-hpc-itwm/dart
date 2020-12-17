#include "http/server.hpp"

#include <boost/bind/bind.hpp>
#include <functional>
#include <stdexcept>

using namespace http;

server::server(net::io_pool& pool, unsigned short port, const boost::filesystem::path & certs)
: _pool(pool)
, _acceptor(pool.get_service(), net::endpoint(boost::asio::ip::tcp::v4(), port))
, _context(pool.get_service(), boost::asio::ssl::context::sslv23)
{
  _context.set_options(boost::asio::ssl::context::default_workarounds
    | boost::asio::ssl::context::no_sslv2
    | boost::asio::ssl::context::single_dh_use);
  _context.use_certificate_chain_file((certs / "server.crt").generic_string());
  _context.use_private_key_file((certs / "server.key").generic_string(), boost::asio::ssl::context::pem);
  _context.use_tmp_dh_file((certs / "dh2048.pem").generic_string());

  _acceptor.set_option(net::acceptor::reuse_address(true));
}

void server::run()
{
  start_accept();
}

void server::start_accept()
{
  client* new_client = new client(get_service(), get_context(), true);
  _acceptor.async_accept(new_client->socket(),
    boost::bind(&server::handle_accept, this, new_client,
      boost::asio::placeholders::error));
}

void server::handle_accept(client* client, const boost::system::error_code& error)
{
  if (!error)
  {
    client->await_requests(std::bind(&server::handle_message, this, 
      std::placeholders::_1, std::placeholders::_2));
  }
  else
  {
    delete client;
  }

  start_accept();
}