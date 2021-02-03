#include <iostream>
#include <functional>
#include <boost/program_options.hpp>

#include "dart_server.hpp"

#include "log/log.hpp"

#include <thread>

namespace
{
  class cout_logger final : public log_message::log_listener
  {
  public:
    virtual void log_info(const std::string& message) override
    {
      std::cout << "[INFO ] " << std::this_thread::get_id() << ": " <<  message << std::endl;
    }
    virtual void log_error(const std::string& message) override
    {
      std::cout << "[ERROR] " << std::this_thread::get_id() << ": " << message << std::endl;
    }
  };
}

int main(int argc, char* argv[])
{
  try
  {
    cout_logger logger;
    log_message::register_loglistener(&logger);

    namespace po = boost::program_options;

    unsigned short port = 7777;
    size_t pool_size = 1;

    po::options_description desc("Allowed options");
    desc.add_options()
      ("help", "produces help message")
      ("port", po::value<unsigned short>(&port)->default_value(7777), "sets the port")
      ("io-pool-size", po::value<size_t>(&pool_size)->default_value(1), "number of threads in io pool")
      ("gspc-ssh-private-key", po::value<std::string>(), "ssh private key for gpispace")
      ("gspc-ssh-public-key", po::value<std::string>(), "ssh public key for gpispace")
      ("gspc-ssh-username", po::value<std::string>(), "ssh username for gpispace")
      ("gspc-ssh-port", po::value<std::string>(), "ssh port for gpispace")
      ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
      std::cout << desc << "\n";
      return 1;
    }
    
    net::io_pool pool(pool_size);

    boost::property_tree::ptree config;
    config.put("port", port);
    if (vm.count("gspc-ssh-private-key")) config.put("gspc.ssh_private_key", vm["gspc-ssh-private-key"].as<std::string>());
    if (vm.count("gspc-ssh-public-key")) config.put("gspc.ssh_public_key", vm["gspc-ssh-public-key"].as<std::string>());
    if (vm.count("gspc-ssh-username")) config.put("gspc.ssh_username", vm["gspc-ssh-username"].as<std::string>());
    if (vm.count("gspc-ssh-port")) config.put("gspc.ssh_port", vm["gspc-ssh-port"].as<std::string>());


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