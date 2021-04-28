#pragma once

#include <string>

namespace http
{
  /**
  * A class to construct and parse valid url's.
  */
  class url final
  {
  public:
    /**
    * Default constructor.
    */
    url() = default;

    /**
    * Constructs an url from a resource and a query.
    *
    * @param resource the resource
    * @param query    the query
    */
    url(const std::string& resource, const std::string& query)
      : path(resource)
      , query(query)
    {
    }

    /**
    * Constructs an url from a string.
    *
    * Parses the string and splits it in path and query.
    *
    * @param full_url the string
    */
    explicit url(const std::string& full_url)
    {
      auto i = full_url.find_first_of("?");
      path = full_url.substr(0, i);
      query = full_url.substr(i+1);

      // todo verify path & query
    }

    /**
    * Converts the url to a string.
    *
    * @return the url string.
    */
    std::string to_string() const
    {
      return path + "?" + query;
    }

    std::string path;
    std::string query;
  };
}