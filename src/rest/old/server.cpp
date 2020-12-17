#include "rest/server.hpp"
#include <boost/property_tree/json_parser.hpp>

#include <stdexcept>

using namespace rest;

server::server(net::io_pool& pool, unsigned short port, router&& router)
: _pool(pool)
, _acceptor(pool.get_service(), net::endpoint(boost::asio::ip::tcp::v4(), port))
, _context(pool.get_service(), boost::asio::ssl::context::sslv23)
, _router(std::move(router))
{
  _context.set_options(boost::asio::ssl::context::default_workarounds
    | boost::asio::ssl::context::no_sslv2
    | boost::asio::ssl::context::single_dh_use);
  _context.use_certificate_chain_file("../certs/server.crt");
  _context.use_private_key_file("../certs/server.key", boost::asio::ssl::context::pem);
  _context.use_tmp_dh_file("../certs/dh2048.pem");

  _acceptor.set_option(net::acceptor::reuse_address(true));
}

void server::run()
{
  start_accept();
}

http::response server::process(http::request&& request) const
{
  auto method = request.method();
  auto result = _router.handle_request(std::move(request));

  // 1xx, 204, 304 response and any response to HEAD request must not have a Content-Length
  http::response response(result.status_code);
  if (result.status_code >= 100 && result.status_code < 200)
    return response;
  if (result.status_code == 204 || result.status_code == 304)
    return response;
  if (method == "HEAD")
    return response;

  std::stringstream stream;
  boost::property_tree::json_parser::write_json(stream, result.content, false);
  auto string = stream.str();

  response["Content-Length"] = std::to_string(string.size());
  response.set_body(std::vector<char>(string.begin(), string.end()));

  return response;
}


void server::start_accept()
{
  session* new_session = new session(this);
  _acceptor.async_accept(new_session->socket(),
    boost::bind(&server::handle_accept, this, new_session,
      boost::asio::placeholders::error));
}

void server::handle_accept(session* new_session, const boost::system::error_code& error)
{
  if (!error)
  {
    new_session->start();
  }
  else
  {
    delete new_session;
  }

  start_accept();
}