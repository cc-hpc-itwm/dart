#include "message.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/asio/ip/host_name.hpp>

#include <sstream>

using namespace rest;

/**
* Constructor.
*
* Creates a rest response with the given status and content
*
* @param status  the status of the rest response
* @param content the content of the rest response
*/
message::message(const http::status_code& status, const boost::property_tree::ptree& content)
: _status(status)
, _method("")
, _url("/")
, _content(content)
{
}

/**
* Constructor.
*
* Creates a rest request for a given url with the given method and content.
*
* @param method the request method
* @param url    the url of the resource
*/
message::message(const std::string& method, const http::url& url, const boost::property_tree::ptree& content)
: _status()
, _method(method)
, _url(url)
, _content(content)
{
}

/**
* Converts the rest message to an http message.
*
* @return the http message
*/
http::message message::to_http_message() const noexcept
{
  std::stringstream ss;
  boost::property_tree::json_parser::write_json(ss, content(), false);
  auto str = ss.str();

  std::vector<char> content(str.begin(), str.end());

  if (is_request())
  {
    // Create http request
    http::message request(method(), url());

    request["Host"] = boost::asio::ip::host_name();
    request["Content-Length"] = std::to_string(content.size());
    request["Content-Type"] = "application/json";

    request.set_body(std::move(content));
    return request;
  }
  else
  {
    // Create http response
    http::message response(status());

    response["Host"] = boost::asio::ip::host_name();
    response["Content-Length"] = std::to_string(content.size());
    response["Content-Type"] = "application/json";

    response.set_body(std::move(content));
    return response;
  }
}