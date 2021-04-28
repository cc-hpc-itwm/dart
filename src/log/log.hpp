#pragma once

#include <string>

namespace log_message
{
  /**
  * Small interface for receiving log messages.
  */
  class log_listener
  {
  public:
    virtual ~log_listener() = default;

    virtual void log_info(const std::string& message) = 0;
    virtual void log_error(const std::string& message) = 0;
  };

  /**
  * Notifies all log listeners that an info message has been
  * received.
  *
  * @param message the message
  */
  extern void info(const std::string& message);

  /**
  * Notifies all log listeners that an error message has been
  * received.
  *
  * @param message the message
  */
  extern void error(const std::string& message);

  /**
  * Registers a log listener.
  *
  * Note that it is the responsibility of the user to ensure that
  * the registered listener lives until @see unregister_loglistener
  * is called.
  *
  * @param listener the listener to register
  */
  extern void register_loglistener(log_listener* listener);

  /**
  * Unregisters a log listener.
  *
  * @param listener the listener to unregister
  */
  extern void unregister_loglistener(log_listener* listener);
}