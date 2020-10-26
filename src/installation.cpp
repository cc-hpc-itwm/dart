#include <installation.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <stdexcept>

namespace dart
{
  namespace
  {
    void check ( boost::filesystem::path const& path
               , bool okay
               , std::string const& message
               )
    {
      if (!okay)
      {
        throw std::logic_error
          ( ( boost::format ("%1% %2%: Installation incomplete!?")
            % path
            % message
            ).str()
          );
      }
    }

    void check_is_directory (boost::filesystem::path const& path)
    {
      check ( path
            , boost::filesystem::is_directory (path)
            , "is not a directory"
            );
    }
    void check_is_file (boost::filesystem::path const& path)
    {
      check ( path
            , boost::filesystem::exists (path)
            , "does not exist"
            );
      check ( path
            , boost::filesystem::is_regular_file (path)
            , "is not a regular file"
            );
    }

    //! \todo configure
    boost::filesystem::path gspc_home
      (boost::filesystem::path const& installation_path)
    {
      return installation_path / "libexec" / "bundle" / "gpispace";
    }
    boost::filesystem::path libraries_path
      (boost::filesystem::path const& installation_path)
    {
      return installation_path / "workflow";
    }
    boost::filesystem::path workflow_path
      (boost::filesystem::path const& installation_path)
    {
      return installation_path / "workflow";
    }
    boost::filesystem::path workflow_file
      (boost::filesystem::path const& installation_path)
    {
      return workflow_path (installation_path) / "dart_workflow.pnet";
    }
  }

  installation::installation (boost::filesystem::path const path)
    : _path (path)
  {
    //! \todo more detailed tests!?
    check_is_directory (gspc_home (_path));
    check_is_directory (libraries_path (_path));
    check_is_directory (workflow_path (_path));
    check_is_file (workflow());
  }

  installation::installation()
    : installation (boost::filesystem::canonical ( boost::filesystem::path ("/")
                                          / "proc"
                                          / std::to_string (getpid())
                                          / "exe"
                                          )
                    )
  {}



  boost::filesystem::path installation::workflow() const
  {
    return workflow_file (_path);
  }
  gspc::installation installation::gspc_installation
    (boost::program_options::variables_map& vm) const
  {
    gspc::set_gspc_home (vm, gspc_home (_path));
    gspc::set_application_search_path (vm, libraries_path (_path));

    return {vm};
  }
}
