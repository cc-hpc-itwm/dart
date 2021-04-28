#pragma once

#include <boost/filesystem/path.hpp>

/**
* Manages the installation of the dart infrastructure.
*/
class installation final
{
public:
  /**
  * Determines the root folder for the installation from
  * the install location of the server executable.
  *
  * Checks if all required files can be found, if not throws
  * an error.
  */
  installation();

  /**
  * Checks and sets the install location to the given path.
  */
  installation(const boost::filesystem::path& path);

  boost::filesystem::path workflow() const;
  boost::filesystem::path gspc() const;
  boost::filesystem::path libraries() const;
  boost::filesystem::path certs() const;
  boost::filesystem::path stop_workers_script() const;
private:
  void check_install();
private:
  boost::filesystem::path _home;
};