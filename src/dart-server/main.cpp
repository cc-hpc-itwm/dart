#include <iostream>
#include <functional>

#include "dart_server.hpp"

#include "log/log.hpp"

namespace
{
  class cout_logger final : public log_message::log_listener
  {
  public:

    virtual void log_info(const std::string& message) override
    {
      std::cout << "[INFO ] " <<  message << std::endl;
    }
    virtual void log_error(const std::string& message) override
    {
      std::cout << "[ERROR] " << message << std::endl;
    }
  };
}

int main()
{
  try
  {
    cout_logger logger;
    log_message::register_loglistener(&logger);

    net::io_pool pool(1);

    boost::property_tree::ptree config;
    config.put("port", 7777);

    dart_server server(pool, config, installation());

    pool.run();

    server.run();

    pool.stop_and_wait();
  }
  catch (const std::exception& exception)
  {
    std::cout << exception.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "unknown exception occured" << std::endl;
  }
  return 0;
}