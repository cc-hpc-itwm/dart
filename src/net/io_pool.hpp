#pragma once

#include <thread>
#include <vector>

#include "net/types.hpp"

namespace net
{
  /**
  * A small helper class to create an io pool from which the
  * io services get pulled in a Round-Robin fashion.
  */
  class io_pool final
  {
  public:
    /**
    * Creates an io pool of the given size.
    *
    * @param pool_size the size of the pool
    */
    io_pool(size_t pool_size = 1);

    /**
    * Destructor.
    */
    ~io_pool() = default;

    /**
    * Starts all the services in the io pool so that they are
    * ready to handle io requests.
    */
    void run();

    /**
    * Waits until all services have exited.
    *
    * Blocking call!
    */
    void wait();

    /**
    * Sends a stop command and afterwards waits.
    *
    * Blocking call!
    */
    void stop_and_wait();

    /**
    * Gets the next service that should be used.
    *
    * Services get returned in a Round-Robin fashion.
    *
    * @return the service
    */
    net::io_service& get_service() noexcept;
  private:
    size_t _next = 0;
    std::vector<net::io_service::work> _works;
    std::vector<std::unique_ptr<net::io_service>> _services;
    std::vector<std::thread> _threads;
  };
}