#include "http/response.hpp"

#include <stdexcept>

using namespace http;

/**
* Creates a response.
*
* @param status       the status code
* @param http_version the version of the http protocol
*/
response::response(status_code status, std::string&& http_version)
: _status_code(status)
, _http_version(http_version)
, _response_fields()
, _body()
{
}

/**
* Checks if the response has a given field.
*
* @param name the field to check for
* @return     returns true, if the request has the field
*/
bool response::has_field(const std::string & name) const noexcept
{
  return _response_fields.find(name) != _response_fields.end();
}

/**
* Returns the values of a given field.
*
* Throws an exception if the field is unavailable.
*
* @param name the name of the field
* @return     the contents of the field
*/
const std::string& response::operator[](const std::string& name) const
{
  if (!has_field(name))
    throw std::runtime_error("request does not have field '" + name + "'");
  return _response_fields.at(name);
}

/**
* Returns the values of a given field.
*
* Creates the field if unavailable.
*
* @param name the name of the field
* @return     the contents of the field
*/
std::string& response::operator[](const std::string& name)
{
  return _response_fields[name];
}

/**
* Creates a buffer with the response contents.
*
* @return the buffer
*/
std::vector<char> response::to_buffer() const
{
  std::string response_line = _http_version + " " + std::to_string(_status_code) + " " + status_code_to_string(_status_code) + "\r\n";

  std::vector<char> _content;
  _content.insert(_content.end(), response_line.begin(), response_line.end());
  for (auto& field : _response_fields)
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
