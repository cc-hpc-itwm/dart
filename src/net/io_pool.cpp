#include "net/io_pool.hpp"

#include "log/log.hpp"
#include <stdexcept>

using namespace net;

/**
* Creates an io pool of the given size.
*
* @param pool_size the size of the pool
*/
io_pool::io_pool(size_t pool_size)
{
  if (pool_size == 0)
    throw std::runtime_error("[io_pool::io_pool] pool size must not be zero");

  _services.reserve(pool_size);
  _works.reserve(pool_size);
  for (size_t i = 0; i < pool_size; ++i)
  {
    _services.push_back(std::make_unique<net::io_service>());
    _works.push_back(boost::asio::io_service::work(*_services[i]));
  }
}

/**
* Starts all the services in the io pool so that they are
* ready to handle io requests.
*/
void io_pool::run()
{
  _threads.reserve(_services.size());
  for (auto& service : _services)
    _threads.push_back(std::thread(
      [&service]() {
        while (true)
        {
          try
          {
            service->run();
            return;
          }
          catch (std::exception & exc)
          {
            log_message::error(std::string("Error occured: ") + exc.what());
          }
          catch (...)
          {
            log_message::error(std::string("Unknown error occured"));
          }
        }
      }));
}

/**
* Waits until all services have exited.
*
* Blocking call!
*/
void io_pool::wait()
{
  for (auto& thread : _threads)
    thread.join();
}

/**
* Sends a stop command and afterwards waits.
*
* Blocking call!
*/
void io_pool::stop_and_wait()
{
  for (auto& service : _services)
    service->stop();

  for (auto& thread : _threads)
    thread.join();
}

/**
* Gets the next service that should be used.
*
* Services get returned in a Round-Robin fashion.
*
* @return the service
*/
net::io_service& io_pool::get_service() noexcept
{
  auto i = _next;
  _next = (_next + 1) % _services.size();
  return *_services[i];
}