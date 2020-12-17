#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <optional>

#include "http/status_code.hpp"

namespace http
{
  class response final
  {
  public:
    response() = default;

    /**
    * Creates a response.
    *
    * @param status       the status code
    * @param http_version the version of the http protocol
    */
    response(status_code status, std::string&& http_version = "HTTP/1.1");
    ~response() = default;

    /**
    * Checks if the response has a given field.
    *
    * @param name the field to check for
    * @return     returns true, if the request has the field
    */
    bool has_field(const std::string& name) const noexcept;

    /**
    * Returns the values of a given field.
    *
    * Throws an exception if the field is unavailable.
    *
    * @param name the name of the field
    * @return     the contents of the field
    */
    const std::string& operator[](const std::string& name) const;

    /**
    * Returns the values of a given field.
    *
    * Creates the field if unavailable.
    *
    * @param name the name of the field
    * @return     the contents of the field
    */
    std::string& operator[](const std::string& name);

    /**
    * Gets the message body.
    *
    * @return the message body
    */
    inline const std::vector<char>& get_body() const noexcept
    {
      return _body;
    }

    /**
    * Sets the message body.
    *
    * @param content the message body
    */
    inline void set_body(const std::vector<char>& content)
    {
      _body = content;
    }

    /**
    * Sets the message body.
    *
    * @param content the message body
    */
    inline void set_body(std::vector<char>&& content) noexcept
    {
      _body = std::move(content);
    }

    /**
    * Creates a buffer with the response contents.
    *
    * @return the buffer
    */
    std::vector<char> to_buffer() const;
  private:
    status_code _status_code  = http::status_code_200_OK;
    std::string _http_version = "HTTP/1.1";
    std::unordered_map<std::string, std::string> _response_fields;
    std::vector<char> _body;
  };
}