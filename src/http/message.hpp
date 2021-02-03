#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "http/status_code.hpp"
#include "http/version.hpp"
#include "http/url.hpp"

namespace http
{
  class message final
  {
  public:
    /**
    * Creates an empty, invalid message.
    */
    message() = default;

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
    message(const std::string& part1, const std::string& part2, const std::string& part3);

    /**
    * Creates an http request.
    *
    * @param method  the request method
    * @param url     the request url
    * @param version the http version
    */
    message(const std::string& method, const url& url, const version& http_version = http::version(1, 1));

    /**
    * Creates an http response.
    *
    * @param status  the status code
    * @param version the http version
    */
    explicit message(const status_code& code, const version& http_version = http::version(1, 1));

    /**
    * Creates an http response.
    *
    * @param code    the status code
    * @param message the status message
    * @param version the http version
    */
    message(unsigned short code, const std::string& message, const version& http_version = version(1, 1));

    ~message() = default;

    /**
    * Checks if the message has a given field.
    *
    * @param name the field to check for
    * @return     returns true, if the message has the field
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

    /**
    * Checks if the message is a request.
    *
    * @return true, if the message is a request
    */
    inline bool is_request() const noexcept { return _request; }

    /**
    * Checks if the message is a response.
    *
    * @return true, if the message is a response
    */
    inline bool is_response() const noexcept { return !_request; }

    /**
    * Gets the request method.
    *
    * If the message is a request, returns its request message.
    * If not returns an empty string.
    *
    * @return the request method
    */
    const std::string& get_method() const noexcept { static std::string _empty = ""; return _request ? _part1 : _empty; }

    /**
    * Gets the url.
    *
    * If the message is a request, returns its url.
    * If not returns "/".
    *
    * @return the request method
    */
    url get_url() const { return _request ? http::url(_part2) : http::url("/"); }

    /**
    * Gets the http version.
    *
    * @return the http version
    */
    version get_version() const { return http::version(_request ? _part3 : _part1); }

    /**
    * Gets the status code.
    *
    * If the message is a response, returns its status code.
    * If not returns an invalid status code.
    *
    * @return the status code
    */
    status_code get_status_code() const noexcept { return _request ? http::status_code() : http::status_code(_part2); }

    /**
    * Gets the status message.
    *
    * If the message is a response, returns its status message.
    * If not returns an empty string.
    *
    * @return the status message
    */
    const std::string& get_status_message() const noexcept { static std::string _empty = ""; return _request ? _empty : _part3; }
  private:
    bool _request = false;
    std::string _part1 = "";
    std::string _part2 = "";
    std::string _part3 = "";
    std::unordered_map<std::string, std::string> _header_fields;
    std::vector<char> _body;
  };
}