#include "rest/client.hpp"

#include <boost/asio/ip/host_name.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace rest;

client::client(net::ssl_socket&& socket)
: _socket(std::move(socket))
{
}

response client::send_request(const std::string& method, const std::string& url, const boost::property_tree::ptree& content)
{
  http::request request(std::move(method), std::move(url));

  std::stringstream stream;
  boost::property_tree::json_parser::write_json(stream, content, false);
  auto string = stream.str();

  request["Host"] = boost::asio::ip::host_name();
  request["Content-Length"] = std::to_string(string.size());
  request.set_body(std::vector<char>(string.begin(), string.end()));

  boost::asio::write(_socket, boost::asio::buffer(request.to_buffer()));
}

void client::handle_read(const boost::system::error_code& error,
  size_t bytes_transferred)
{

}

void client::handle_write(const boost::system::error_code& error)
{

}