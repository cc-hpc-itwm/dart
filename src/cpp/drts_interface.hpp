#include <installation.hpp>

#include <drts/certificates.hpp>
#include <drts/information_to_reattach.hpp>
#include <drts/scoped_rifd.hpp>

#include <pnetc/type/config.hpp>
#include <pnetc/type/task_result.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/python.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <forward_list>
#include <queue>
#include <unordered_map>
#include <vector>

namespace
{
  using descriptions_and_entry_points_t
    = std::forward_list< std::pair< gspc::worker_description
                                  , gspc::rifd_entry_points
                                  >
                       >;
}

class drts_wrapper : boost::noncopyable
{
public:
  drts_wrapper
    ( std::string const& anaconda_home
    , boost::python::dict const&
    );

  void start_runtime();
  void start_runtime (std::string const& nodefile);
  void start_runtime
    ( std::string const& machinefile
    , std::size_t workers_per_host
    );

  void add_workers (std::string const& nodefile);
  void add_workers
    ( std::string const& machinefile
    , std::size_t workers_per_host
    , boost::python::list const& capabilities
    , std::size_t shm_size
    );
  void add_workers
    ( boost::python::list const& pyhosts
    , std::size_t workers_per_host
    , boost::python::list const& capabilities
    , std::size_t shm_size
    );

  boost::python::dict remove_workers();
  boost::python::dict remove_workers (boost::python::list const&);
  boost::python::dict remove_workers (std::vector<std::string> const& hosts);

  boost::python::list run
    ( std::string const& method
    , boost::python::list const& locations_and_parameters
    );

  boost::python::list run
    ( std::string const& method
    , boost::python::list const& locations_and_parameters
    , std::string const& output_directory
    );

  boost::python::list run
    ( std::string const& abs_path_to_module
    , std::string const& method
    , boost::python::list const& locations_and_parameters
    );

  boost::python::list run
    ( std::string const& abs_path_to_module
    , std::string const& method
    , boost::python::list const& locations_and_parameters
    , std::string const& output_directory
    );

  gspc::job_id_t async_run
    ( std::string const& method
    , boost::python::list const& locations_and_parameters
    );

  gspc::job_id_t async_run
     ( std::string const& method
     , boost::python::list const& locations_and_parameters
     , std::string const& output_directory
     );

  gspc::job_id_t async_run
     ( std::string const& abs_path_to_module
     , std::string const& method
     , boost::python::list const& locations_and_parameters
     );

  gspc::job_id_t async_run
     ( std::string const& abs_path_to_module
     , std::string const& method
     , boost::python::list const& locations_and_parameters
     , std::string const& output_directory
     );

  boost::python::list collect_results
    (gspc::job_id_t const& job_id);

  unsigned long get_number_of_remaining_tasks
    (gspc::job_id_t const& job);

  unsigned long get_total_number_of_tasks
     (gspc::job_id_t const& job);

  boost::python::object pop_result (gspc::job_id_t const& job);

  void stop_runtime();

private:
  void add_workers (descriptions_and_entry_points_t const&);

  boost::program_options::variables_map get_options
    (boost::python::dict const&);

  std::list<pnet::type::value::value_type> get_location_and_parameters
    (boost::python::list const& locations_and_parameters);

  unsigned long get_number_of_tasks
    (boost::python::list const& locations_and_parameters);

  int get_number_of_workers_with_capability
  	(std::string const& capability);

  descriptions_and_entry_points_t parse_worker_description_file
    (boost::filesystem::path const& nodefile);

  bool known_location (std::string const& location);

  boost::python::object get_next_task_result
     (gspc::client& client, gspc::job_id_t const& job);

private:
  boost::program_options::variables_map _vm;
  gspc::Certificates _certificates;
  dart::installation _installation;
  gspc::installation _gspc_installation;
  std::unique_ptr<gspc::scoped_rifd> _master_rifd;
  std::unique_ptr<gspc::scoped_runtime_system> _drts;
  std::vector<std::unique_ptr<gspc::scoped_rifds>> _worker_rifds;
  boost::filesystem::path _python_home;
  descriptions_and_entry_points_t _descriptions_and_entry_points;
  std::unordered_map<gspc::job_id_t, std::pair<unsigned long, std::queue<boost::python::object>>> _job_results;
  boost::uuids::random_generator _generator;
};
