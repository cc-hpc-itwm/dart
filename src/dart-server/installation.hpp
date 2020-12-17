#pragma once

#include <boost/filesystem/path.hpp>

class installation final
{
public:
  installation();
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