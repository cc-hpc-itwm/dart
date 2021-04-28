#include "dart-server/installation.hpp"

#include <boost/filesystem/operations.hpp>

#include "log/log.hpp"

#include <sys/types.h>
#include <unistd.h>

/**
* Determines the root folder for the installation from
* the install location of the server executable.
*
* Checks if all required files can be found, if not throws
* an error.
*/
installation::installation()
: _home(boost::filesystem::canonical(boost::filesystem::path("/") 
  / "proc"
  / std::to_string(getpid())
  / "exe").parent_path().parent_path() )
{
  check_install();
}

/**
* Checks and sets the install location to the given path.
*/
installation::installation(const boost::filesystem::path& path)
: _home(path)
{
  check_install();
}

boost::filesystem::path installation::workflow() const
{
  return _home / "workflow" / "dart_workflow.pnet";
}

boost::filesystem::path installation::gspc() const
{
  return _home / "libexec" / "bundle" / "gpispace";
}

boost::filesystem::path installation::libraries() const
{
  return _home / "workflow";
}

boost::filesystem::path installation::certs() const
{
  return _home / "certs";
}

boost::filesystem::path installation::stop_workers_script() const
{
  return _home / "bin" / "stop_workers.sh";
}

void installation::check_install()
{
  log_message::info("[installation::check_install] home is '" + _home.generic_string() + "'");
  if (!boost::filesystem::is_regular_file(workflow()))
    throw std::logic_error("[installation::check_install] workflow not found");
  if (!boost::filesystem::is_directory(gspc()))
    throw std::logic_error("[installation::check_install] gspc not found");
  if (!boost::filesystem::is_directory(libraries()))
    throw std::logic_error("[installation::check_install] libraries not found");
  if (!boost::filesystem::is_directory(certs()))
    throw std::logic_error("[installation::check_install] certs not found");
  if (!boost::filesystem::is_regular_file(stop_workers_script()))
    throw std::logic_error("[installation::check_install] stop_workers not found");
}