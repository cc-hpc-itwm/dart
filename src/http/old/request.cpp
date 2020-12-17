#include "http/request.hpp"

#include <stdexcept>
#include <iostream>

using namespace http;

/**
* Creates a request for the resource.
*
* @param method       the request method
* @param resource     the resource
* @param http_version the version of the http protocol
*/
request::request(std::string&& method, std::string&& resource, std::string&& http_version)
: _request_method(method)
, _resource(resource)
, _http_version(http_version)
, _request_fields()
, _body()
{

}

/**
* Checks if the request has a given field.
*
* @param name the field to check for
* @return     returns true, if the request has the field
*/
bool request::has_field(const std::string& name) const noexcept
{
  return _request_fields.find(name) != _request_fields.end();
}

/**
* Returns the values of a given field.
*
* Throws an exception if the field is unavailable.
*
* @param name the name of the field
* @return     the contents of the field
*/
const std::string& request::operator[](const std::string& name) const
{
  if (!has_field(name))
    throw std::runtime_error("request does not have field '" + name + "'");
  return _request_fields.at(name);
}

/**
* Returns the values of a given field.
*
* Creates the field if unavailable.
*
* @param name the name of the field
* @return     the contents of the field
*/
std::string& request::operator[](const std::string& name)
{
  return _request_fields[name];
}

/**
* Creates a buffer with the request contents.
*
* @return the buffer
*/
std::vector<char> request::to_buffer() const
{
  std::string request_line = _request_method + " " + _resource + " " + _http_version + "\r\n";

  std::vector<char> _content;
  _content.insert(_content.end(), request_line.begin(), request_line.end());
  for (auto& field : _request_fields)
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
