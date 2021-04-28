#pragma once

#include <string>
#include <stdexcept>

namespace http
{
  /**
  * Constructs and parses http versions.
  */
  class version final
  {
  public:
    /**
    * Default Constructor.
    *
    * The http version is 1.1
    */
    version() = default;

    /**
    * Constructs an http version from a major and minor number.
    *
    * The http version will be "[major].[minor]".
    *
    * @param major the major
    * @param minor the minor
    */
    version(unsigned major, unsigned minor)
    : _major(major)
    , _minor(minor)
    {
    }

    /**
    * Constructs an http version from a string.
    *
    * Atm only "HTTP/1.1" and "HTTP/1.0" are allowed.
    *
    * @param version the version string.
    */
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

    /**
    * Converts the major and minor numbers to a version string.
    *
    * @return the version string.
    */
    std::string to_string() const
    { 
      return "HTTP/" + std::to_string(_major) + "." + std::to_string(_minor); 
    }
  private:
    unsigned _major = 1;
    unsigned _minor = 1;
  };
}