#pragma once

#include <drts/drts.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>

namespace dart
{
  //! \note collects information relative to the path of the executable
  class installation
  {
  public:
    installation();
    installation (boost::filesystem::path const);

    boost::filesystem::path workflow() const;
    gspc::installation gspc_installation
      (boost::program_options::variables_map&) const;
    //! \todo remove
    boost::filesystem::path const& home() const
    {
      return _path;
    }

  private:
    boost::filesystem::path const _path;
  };
}
