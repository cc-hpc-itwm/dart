#pragma once

#include <thread>
#include <vector>

#include "net/types.hpp"

namespace net
{
  class io_pool final
  {
  public:
    io_pool(size_t pool_size = 1);
    ~io_pool() = default;

    void run();

    void wait();
    void stop_and_wait();

    net::io_service& get_service() noexcept;
  private:
    size_t _next = 0;
    std::vector<net::io_service::work> _works;
    std::vector<std::unique_ptr<net::io_service>> _services;
    std::vector<std::thread> _threads;
  };
}