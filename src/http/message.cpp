#include "http/message.hpp"

#include <stdexcept>
#include <iostream>

using namespace http;

/**
* Creates a message from a start line.
*
* A http message has a start line that contains three different
* space seperated parts.
*
* Depending on the parts, determines if the message is a request
* or a response. If it is neither, the function throws an error.
*
* @param part1 the first part of the status line
* @param part2 the second part of the status line
* @param part3 the last part of the status line
*/
message::message(const std::string& part1, const std::string& part2, const std::string& part3)
: _part1(part1)
, _part2(part2)
, _part3(part3)
{
  if (part3.compare(0, 5, "HTTP/") == 0)
    _request = true;
  else
    _request = false;
}

/**
* Creates an http request.
*
* @param method  the request method
* @param url     the request url
* @param version the http version
*/
message::message(const std::string& method, const http::url& url, const http::version& http_version)
: _request(true)
, _part1(method)
, _part2(url.to_string())
, _part3(http_version.to_string())
{
}

/**
* Creates an http response.
*
* @param status  the status code
* @param version the http version
*/
message::message(const http::status_code& code, const http::version& http_version)
: message(code.code(), code.message(), http_version)
{
}

/**
* Creates an http response.
*
* @param code    the status code
* @param message the status message
* @param version the http version
*/
message::message(unsigned short code, const std::string& message, const http::version& http_version)
: _request(false)
, _part1(http_version.to_string())
, _part2(std::to_string(code))
, _part3(message)
{

}

/**
* Checks if the message has a given field.
*
* @param name the field to check for
* @return     returns true, if the message has the field
*/
bool message::has_field(const std::string& name) const noexcept
{
  return _header_fields.find(name) != _header_fields.end();
}

/**
* Returns the values of a given field.
*
* Throws an exception if the field is unavailable.
*
* @param name the name of the field
* @return     the contents of the field
*/
const std::string& message::operator[](const std::string& name) const
{
  if (!has_field(name))
    throw std::runtime_error("[http::message::operator[]] no field named '" + name + "'");
  return _header_fields.at(name);
}

/**
* Returns the values of a given field.
*
* Creates the field if unavailable.
*
* @param name the name of the field
* @return     the contents of the field
*/
std::string& message::operator[](const std::string& name)
{
  return _header_fields[name];
}

/**
* Creates a buffer with the request contents.
*
* @return the buffer
*/
std::vector<char> message::to_buffer() const
{
  std::string status_line = _part1 + " " + _part2 + " " + _part3 + "\r\n";

  std::vector<char> _content;
  _content.insert(_content.end(), status_line.begin(), status_line.end());
  for (auto& field : _header_fields)
  {
    _content.insert(_content.end(), field.first.begin(), field.first.end());
    _content.insert(_content.end(), { ':', ' ' });
    _content.insert(_content.end(), field.second.begin(), field.second.end());
    _content.insert(_content.end(), { '\r', '\n' });
  }
  _content.insert(_content.end(), { '\r', '\n' });
  _content.insert(_content.end(), _body.begin(), _body.end());
  return _content;
}
