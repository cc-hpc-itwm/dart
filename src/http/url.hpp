#pragma once

#include <string>

namespace http
{
  class url final
  {
  public:
    url() = default;

    url(const std::string& resource, const std::string& query)
      : path(resource)
      , query(query)
    {
    }

    explicit url(const std::string& full_url)
    {
      auto i = full_url.find_first_of("?");
      path = full_url.substr(0, i);
      query = full_url.substr(i+1);

      // todo verify path & query
    }

    std::string to_string() const
    {
      return path + "?" + query;
    }

    std::string path;
    std::string query;
  };
}