#pragma once

#include <boost/algorithm/string.hpp>
#include <string>

namespace dart
{
  /**
  * A helper function to replace all substrings in a string by the given replacement.
  */
  void replace_substr(std::string& base, const std::string& substr, const std::string& replacement)
  {
    std::size_t index = 0;
    while ((index = base.find(substr, index)) != std::string::npos)
    {
      base.replace(index, substr.size(), replacement);
      index += replacement.size();
    }
  }

  /**
  * Checks if the a given string is the name of the worker.
  *
  * Note that the worker names use the following encoding:
  *  Let <name> be the name of the worker, then it receives the prefix
  *  ":dartname::" and the postfix "::". Afterwards, the following symbols
  *  get replaces:
  *    1) : by :0
  *    2) + by :1
  *    3) # by :2
  *    4) . by :3
  *    5) - by :4
  *  This is necessary because gpispace alters the names and we want to recover
  *  the original name.
  *
  * @return returns the converted name or an empty string.
  */
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

  /**
  * Tries to convert the internal gpispace worker name to the
  * name under which the worker was registered. First tries the
  * dart way, if unsuccessful constructs a best bet.
  *
  * @param internal_worker_name the internal worker name
  * @return the worker name
  */
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
