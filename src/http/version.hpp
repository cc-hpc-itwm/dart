#pragma once

#include <string>
#include <stdexcept>

namespace http
{
  class version final
  {
  public:
    version() = default;
    version(unsigned major, unsigned minor)
    : _major(major)
    , _minor(minor)
    {
    }

    explicit version(const std::string& version)
    {
      if (version == "HTTP/1.1")
      {
        _major = 1; _minor = 1;
      }
      else if (version == "HTTP/1.0")
      {
        _major = 1; _minor = 0;
      }
      else
        throw std::runtime_error("[http::version::version] invalid or unknown version string '" + version + "'");
    }

    version(const version&) = default;
    version(version&&) = default;

    version& operator=(const version&) = default;
    version& operator=(version&&) = default;

    ~version() = default;

    std::string to_string() const
    { 
      return "HTTP/" + std::to_string(_major) + "." + std::to_string(_minor); 
    }
  private:
    unsigned _major = 1;
    unsigned _minor = 1;
  };
}