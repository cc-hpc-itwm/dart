#pragma once

#include <string>
#include <fstream>
#include <boost/dll.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio/ip/host_name.hpp>

#include "workflow/get_public_worker_name.hpp"

namespace dart
{
  static bool already_loaded = false;
  struct
  {
    std::string name;            //!< The name of the worker
    std::string host;            //!< The host name
    std::string python_home;     //!< Path to the python home
    std::string module_prefix;   //!< Path to the prefix for the modules
    std::string output_directory;//!< The output directory
  } worker_config;

  static void load_worker_config(const std::string& internal_name)
  {
    if (already_loaded)
      return;
    auto dir = boost::dll::program_location().parent_path();
    // dir = "${DART_HOME}/libexec/bundle/gpispace/libexec/gspc"
    dir = dir / ".." / ".." / ".." / ".." / ".." / "worker";
    // dir = "${DART_HOME}/worker"

    std::ifstream config_file((dir / "worker.json").generic_string());
    boost::property_tree::ptree content;
    boost::property_tree::json_parser::read_json(config_file, content);
        
    worker_config.name = content.get("name", get_public_worker_name(internal_name));
    worker_config.host = content.get("host", boost::asio::ip::host_name());
    worker_config.python_home = content.get("python_home", "");
    worker_config.module_prefix = content.get("module_prefix", "");
    worker_config.output_directory = content.get("output_directory", "/var/tmp/");

    // All paths are relative to the config file
    worker_config.python_home = boost::filesystem::canonical(worker_config.python_home, dir).generic_string();
    worker_config.module_prefix = boost::filesystem::canonical(worker_config.module_prefix, dir).generic_string();
    worker_config.output_directory = boost::filesystem::canonical(worker_config.output_directory, dir).generic_string();

    already_loaded = true;
  }
}
