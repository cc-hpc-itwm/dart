#include "log/log.hpp"

#include <vector>
#include <mutex>
#include <algorithm>

namespace 
{
  std::vector<log_message::log_listener*> _listeners;
  std::mutex _listener_mutex;
}

void log_message::info(const std::string& message)
{
  const std::lock_guard<std::mutex> lock(_listener_mutex);
  for (auto& listener : _listeners)
    listener->log_info(message);
}

void log_message::error(const std::string& message)
{
  const std::lock_guard<std::mutex> lock(_listener_mutex);
  for (auto& listener : _listeners)
    listener->log_error(message);
}

void log_message::register_loglistener(log_message::log_listener* listener)
{
  const std::lock_guard<std::mutex> lock(_listener_mutex);
  _listeners.push_back(listener);
}

void log_message::unregister_loglistener(log_message::log_listener* listener)
{
  const std::lock_guard<std::mutex> lock(_listener_mutex);
  auto iter = std::find(_listeners.begin(), _listeners.end(), listener);
  if (iter != _listeners.end())
    _listeners.erase(iter);
}