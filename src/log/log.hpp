#pragma once

#include <string>

namespace log_message
{
  class log_listener
  {
  public:
    virtual ~log_listener() = default;

    virtual void log_info(const std::string& message) = 0;
    virtual void log_error(const std::string& message) = 0;
  };

  extern void info(const std::string& message);
  extern void error(const std::string& message);

  extern void register_loglistener(log_listener* listener);
  extern void unregister_loglistener(log_listener* listener);
}