#pragma once

#include "http/message.hpp"

#include <boost/property_tree/ptree.hpp>

namespace rest
{
  /**
  * A rest message is either a request or a response.
  *
  * The message will have json-formatted content.
  */
  class message final
  {
  public:
    /**
    * Creates an empty invalid message.
    * Should only be used as placeholder.
    */
    message() = default;
    
    /**
    * Constructor.
    *
    * Creates a rest response with the given status and content
    *
    * @param status  the status of the rest response
    * @param content the content of the rest response
    */
    message(const http::status_code& status, const boost::property_tree::ptree& content = {});

    /**
    * Constructor.
    *
    * Creates a rest request for a given url with the given method and content.
    *
    * @param method the request method
    * @param url    the url of the resource
    */
    message(const std::string& method, const http::url& url, const boost::property_tree::ptree& content = {});

    /**
    * Copy constructor.
    */
    message(const message&) = default;
    
    /**
    * Move constructor.
    */
    message(message&&) = default;

    /**
    * Copy operator.
    */
    message& operator= (const message&) = default;

    /**
    * Move operator
    */
    message& operator= (message&&) = default;

    /**
    * Destructor.
    */
    ~message() = default;

    /**
    * Converts the rest message to an http message.
    *
    * @return the http message
    */
    http::message to_http_message() const noexcept;

    /**
    * Conversion operator to http message.
    *
    * @return the http message
    */
    inline operator http::message() const noexcept { return to_http_message(); }

    /**
    * Checks whether the message is a request.
    *
    * @return true, if the message is a request
    */
    inline bool is_request() const noexcept { return !_status.is_valid(); }

    /**
    * Checks whether the message is a response.
    *
    * @return true, if the message is a response.
    */
    inline bool is_response() const noexcept { return _status.is_valid(); }

    /**
    * Gets the http status code.
    *
    * @return the status code
    */
    inline const http::status_code& status() const noexcept { return _status; }

    /**
    * Gets the http request method.
    *
    * @return the method
    */
    inline const std::string& method() const noexcept { return _method; }

    /**
    * Gets the http request url.
    *
    * @return the url
    */
    inline const http::url& url() const noexcept { return _url; }

    /**
    * Gets the content.
    *
    * @return the content
    */
    inline const boost::property_tree::ptree& content() const noexcept { return _content; }

    /**
    * Gets the content.
    *
    * @return the content
    */
    inline boost::property_tree::ptree& content() noexcept { return _content; }
  private:
    http::status_code _status;
    std::string _method;
    http::url _url;
    boost::property_tree::ptree _content;
  };
}