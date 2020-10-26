#pragma once

#include <boost/algorithm/string.hpp>

namespace dart
{
  std::string get_public_worker_name
    (std::string const& internal_worker_name)
  {
    std::vector<std::string> parts;
    boost::algorithm::split
      (parts, internal_worker_name, boost::algorithm::is_any_of(" "));
    return parts[0] + parts[2].substr (parts[2].find ("-"));
  }
}
