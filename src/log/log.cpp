#include "log/log.hpp"

#include <vector>
#include <mutex>
#include <algorithm>

namespace 
{
  std::vector<log_message::log_listener*> _listeners;
  std::mutex _listener_mutex;
}

/**
* Notifies all log listeners that an info message has been
* received.
*
* @param message the message
*/
void log_message::info(const std::string& message)
{
  const std::lock_guard<std::mutex> lock(_listener_mutex);
  for (auto& listener : _listeners)
    listener->log_info(message);
}

/**
* Notifies all log listeners that an error message has been
* received.
*
* @param message the message
*/
void log_message::error(const std::string& message)
{
  const std::lock_guard<std::mutex> lock(_listener_mutex);
  for (auto& listener : _listeners)
    listener->log_error(message);
}

/**
* Registers a log listener.
*
* Note that it is the responsibility of the user to ensure that
* the registered listener lives until @see unregister_loglistener
* is called.
*
* @param listener the listener to register
*/
void log_message::register_loglistener(log_message::log_listener* listener)
{
  const std::lock_guard<std::mutex> lock(_listener_mutex);
  _listeners.push_back(listener);
}

/**
* Unregisters a log listener.
*
* @param listener the listener to unregister
*/
void log_message::unregister_loglistener(log_message::log_listener* listener)
{
  const std::lock_guard<std::mutex> lock(_listener_mutex);
  auto iter = std::find(_listeners.begin(), _listeners.end(), listener);
  if (iter != _listeners.end())
    _listeners.erase(iter);
}