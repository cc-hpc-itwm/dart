#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <optional>

#include "http/status_code.hpp"

namespace http
{
  class request final
  {
  public:
    /**
    * Creates an empty get request.
    */
    request() = default;

    /**
    * Creates a request for the resource.
    *
    * @param method       the request method
    * @param resource     the resource
    * @param http_version the version of the http protocol
    */
    request(std::string&& method, std::string&& resource, std::string&& http_version = "HTTP/1.1");
    ~request() = default;

    /**
    * Checks if the request has a given field.
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
    * Creates a buffer with the request contents.
    *
    * @return the buffer
    */
    std::vector<char> to_buffer() const;

    const std::string& method() const noexcept { return _request_method; }
    const std::string& resource() const noexcept { return _resource; }
    const std::string& version() const noexcept { return _http_version; }
  private:
    std::string _request_method = "GET";
    std::string _resource       = "/";
    std::string _http_version   = "HTTP/1.1";
    std::unordered_map<std::string, std::string> _request_fields;
    std::vector<char> _body;
  };
}