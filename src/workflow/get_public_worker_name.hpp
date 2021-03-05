#pragma once

#include <boost/algorithm/string.hpp>
#include <string>

namespace dart
{
  void replace_substr(std::string& base, const std::string& substr, const std::string& replacement)
  {
    std::size_t index = 0;
    while ((index = base.find(substr, index)) != std::string::npos)
    {
      base.replace(index, substr.size(), replacement);
      index += replacement.size();
    }
  }

  std::string get_dartname(const std::string& string)
  {
    auto index = string.find(":dartname::");
    auto end = string.find("::", index + 11);

    auto name = string.substr(index + 11, (end - index - 11));
    replace_substr(name, ":4", "-");
    replace_substr(name, ":3", ".");
    replace_substr(name, ":2", "#");
    replace_substr(name, ":1", "+");
    replace_substr(name, ":0", ":");
    return name;
  }

  std::string get_public_worker_name
    (std::string const& internal_worker_name)
  {
    auto name = get_dartname(internal_worker_name);
    if (name != "")
      return name;
    std::vector<std::string> parts;
    boost::algorithm::split
      (parts, internal_worker_name, boost::algorithm::is_any_of(" "));
    return parts[0] + parts[2].substr (parts[2].find ("-"));
  }
}
