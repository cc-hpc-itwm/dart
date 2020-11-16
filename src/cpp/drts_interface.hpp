#pragma once

#include <installation.hpp>

#include <drts/certificates.hpp>
#include <drts/information_to_reattach.hpp>
#include <drts/scoped_rifd.hpp>

#include <pnetc/type/config.hpp>
#include <pnetc/type/task_result.hpp>

#include <cpp/result_storage.hpp>

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
#include <memory>

namespace
{
  using descriptions_and_entry_points_t
    = std::forward_list< std::pair< gspc::worker_description
                                  , gspc::rifd_entry_points
                                  >
                       >;
}

/**
* This class wraps functions from gpispace and will manage the python binding.
*
* This class provides functions that wrap the functions from gpispace. The
* distributed runtime system (drts) uses ssh to communicate with its workers
* and can send them jobs in form of python methods that shall be executed.
* Each worker gets associated with some capabilities for instance a location.
* It is possible to distribute the python jobs to workers with a given location.
*/
class drts_wrapper : boost::noncopyable
{
public:
  /**
  * Constructor.
  *
  * The runtime needs a path to a valid python installation and options to
  * start gpispace. These options are stored in a python dictionary. Required
  * options are ssh_username, ssh_port, ssh_public_key, ssh_private_key to start
  * the ssh server.
  *
  * During the constructor, a rifd (the master) on the server. However, the
  * runtime will not yet be started. A seperate call to drts_wrapper::start_runtime
  * is required.
  *
  * @param python_home The home directory of the python installation
  * @param user_opt    The options that get forwarded to gpispace
  */
  drts_wrapper
    ( std::string const& python_home
    , boost::python::dict const& user_opt
    );

  /**
  * Starts the runtime.
  */
  void start_runtime();

  /**
  * Starts the runtime and adds workers.
  *
  * Starts the runtime and adds workers from a nodefile. For a description
  * of the nodefile format, see drts_wrapper::add_workers.
  *
  * @param nodefile path to a nodefile that contains the worker description
  */
  void start_runtime (std::string const& nodefile);

  /**
  * Starts the runtime and adds workers.
  *
  * Starts the runtime and adds workers_per_host workers for each host
  * specified in the machinefile. For a description of the machinefile format,
  * see drts_wrapper::add_workers.
  *
  * @param machinefile path to a machinefile that contains the worker description
  * @param workers_per_host
  */
  void start_runtime
    ( std::string const& machinefile
    , std::size_t workers_per_host
    );

  /**
  * Adds workers to the distributed runtime system.
  *
  * Reads a nodefile and adds workers based on the information gathered from
  * this file.
  *
  * A nodefile describes has json file format and describes the workers in the
  * following way. Each element in the object describes workers. Each description
  * is a again an object that has four different elements. They are capabilities,
  * num_per_node, shm_size, and hosts.
  * The "capabilities" element is a list of strings, each describing a capability
  * of the worker, for instance it's location.
  * The "num_per_node" element is an integer, the number of workers per host.
  * The "shm_size" element is an integer.
  * The "hosts" element is a list of strings, each being the name of a host.
  *
  * Example : starting 5 workers of type "A" at the location "Frankfurt" on the hosts
  * "host1.baz.de" and  "host2.baz.de" and 10 workers of type "B" at the location
  * "Dublin" on the host "host3.baz.ie" can be achieved with the following nodefile:
  * @code{.json}
  *  {  "worker0" : { "capabilities": ["A", "Frankfurt"]
  *                 , "num_per_node": "5"
  *                 , "shm_size": "0"
  *                 , "hosts" : ["host1.baz.de", "host2.baz.de"]
  *                 },
  *     "worker1" : { "capabilities": ["B", "Dublin"]
  *                 , "num_per_node": "10"
  *                 , "shm_size": "0"
  *                 , "hosts" : ["host3.baz.ie"]
  *                 }
  *  }
  * @endcode
  *
  * @param nodefile path to a nodefile that contains the worker description
  */
  void add_workers (std::string const& nodefile);

  /**
  * Adds workers to the distributed runtime system.
  *
  * Reads a machinefile and adds workers based on the information gathered from
  * this file and the other arguments of the function.
  *
  * Each line in the machinefile is the name of a host.
  *
  * @param machinefile      path to a machinefile that contains the hosts
  * @param workers_per_host the number of workers per host
  * @param capabilities     the capabilities of the workers, e.g., location and type
  * @param shm_size         the shm size
  */
  void add_workers
    ( std::string const& machinefile
    , std::size_t workers_per_host
    , boost::python::list const& capabilities
    , std::size_t shm_size
    );

  /**
  * Adds workers to the distributed runtime system.
  *
  * Spawns workers_per_host workers on each host given in the list pyhosts
  * with the capabilities and the shm_size specified by the arguments of
  * the function.
  *
  * @param pyhosts          a list of hosts
  * @param workers_per_host the number of workers per host
  * @param capabilities     the capabilities of the workers, e.g., location and type
  * @param shm_size         the shm size
  */
  void add_workers
    ( boost::python::list const& pyhosts
    , std::size_t workers_per_host
    , boost::python::list const& capabilities
    , std::size_t shm_size
    );

  /**
  * Removes all workers.
  *
  * Tries to remove all workers and returns a dictionary of errors that
  * arise during this process.
  *
  * @return a dictionary of errors
  */
  boost::python::dict remove_workers();

  /**
  * Removes all workers on the specified hosts.
  *
  * Tries to remove all workers on the specified hosts and returns a
  * dictionary of errors that arise during this process.
  *
  * @param pyhosts a python list of the hosts
  * @return        a dictionary of errors
  */
  boost::python::dict remove_workers (boost::python::list const& pyhosts);

  /**
  * Removes all workers on the specified hosts.
  *
  * Tries to remove all workers on the specified hosts and returns a
  * dictionary of errors that arise during this process.
  *
  * @param hosts   a list of the hosts
  * @return        a dictionary of errors
  */
  boost::python::dict remove_workers (std::vector<std::string> const& hosts);

  /**
  * Executes a job on the workers.
  *
  * Executes the python function specified by method in the module <install>/lib/dart_task.py
  * where <install> is the install directory of dart. The method is executed with the given
  * parameters on those workers that have the location as one of their capabilities. The
  * location and parameters list has the following structure:
  * @code{.json}
  * [ {'location' : 'location0', 'parameters': [p00, p01, ...]}
  * , {'location' : 'location1', 'parameters': [p10, p11, ...]}
  * , ...
  * , {'location' : 'locationn', 'parameters': [pn0, pn1, ...]}
  * ]
  * @endcode
  *
  * 
  * @param method                   the python method that shall be executed
  * @param locations_and_parameters the locations and parameters as specified above
  * @return                         the results of the executed method
  */
  boost::python::list run
    ( std::string const& method
    , boost::python::list const& locations_and_parameters
    );

  /**
  * Executes a job on the workers.
  *
  * Executes the python function specified by method in the module <install>/lib/dart_task.py
  * where <install> is the install directory of dart. The method is executed with the given
  * parameters on those workers that have the location as one of their capabilities. The
  * location and parameters list has the following structure:
  * @code{.json}
  * [ {'location' : 'location0', 'parameters': [p00, p01, ...]}
  * , {'location' : 'location1', 'parameters': [p10, p11, ...]}
  * , ...
  * , {'location' : 'locationn', 'parameters': [pn0, pn1, ...]}
  * ]
  * @endcode
  *
  *
  * @param method                   the python method that shall be executed
  * @param locations_and_parameters the locations and parameters as specified above
  * @param output_directory         the directory to store the logs in
  * @return                         the results of the executed method
  */
  boost::python::list run
    ( std::string const& method
    , boost::python::list const& locations_and_parameters
    , std::string const& output_directory
    );

  /**
  * Executes a job on the workers.
  *
  * Executes the python function specified by method in the specified module. The method
  * is executed with the given parameters on those workers that have the location as one
  * of their capabilities. The location and parameters list has the following structure:
  * @code{.json}
  * [ {'location' : 'location0', 'parameters': [p00, p01, ...]}
  * , {'location' : 'location1', 'parameters': [p10, p11, ...]}
  * , ...
  * , {'location' : 'locationn', 'parameters': [pn0, pn1, ...]}
  * ]
  * @endcode
  *
  *
  * @param abs_path_to_module       the absolute path to the python module
  * @param method                   the python method that shall be executed
  * @param locations_and_parameters the locations and parameters as specified above
  * @return                         the results of the executed method
  */
  boost::python::list run
    ( std::string const& abs_path_to_module
    , std::string const& method
    , boost::python::list const& locations_and_parameters
    );

  /**
  * Executes a job on the workers.
  *
  * Executes the python function specified by method in the specified module. The method
  * is executed with the given parameters on those workers that have the location as one
  * of their capabilities. The location and parameters list has the following structure:
  * @code{.json}
  * [ {'location' : 'location0', 'parameters': [p00, p01, ...]}
  * , {'location' : 'location1', 'parameters': [p10, p11, ...]}
  * , ...
  * , {'location' : 'locationn', 'parameters': [pn0, pn1, ...]}
  * ]
  * @endcode
  *
  *
  * @param abs_path_to_module       the absolute path to the python module
  * @param method                   the python method that shall be executed
  * @param locations_and_parameters the locations and parameters as specified above
  * @param output_directory         the directory to store the logs in
  * @return                         the results of the executed method
  */
  boost::python::list run
    ( std::string const& abs_path_to_module
    , std::string const& method
    , boost::python::list const& locations_and_parameters
    , std::string const& output_directory
    );

  /**
  * Executes a job on the workers asynchronously.
  *
  * For a description of the method see drts_wrapper::run. Contrary to synchronous execution
  * the return value of the asynchronous execution is only an id which can be used
  * to later retrieve the results.
  *
  * @return the id of the executed job.
  */
  gspc::job_id_t async_run
    ( std::string const& method
    , boost::python::list const& locations_and_parameters
    );

  /**
  * Executes a job on the workers asynchronously.
  *
  * For a description of the method see drts_wrapper::run. Contrary to synchronous execution
  * the return value of the asynchronous execution is only an id which can be used
  * to later retrieve the results.
  *
  * @return the id of the executed job.
  */
  gspc::job_id_t async_run
     ( std::string const& method
     , boost::python::list const& locations_and_parameters
     , std::string const& output_directory
     );

  /**
  * Executes a job on the workers asynchronously.
  *
  * For a description of the method see drts_wrapper::run. Contrary to synchronous execution
  * the return value of the asynchronous execution is only an id which can be used
  * to later retrieve the results.
  *
  * @return the id of the executed job.
  */
  gspc::job_id_t async_run
     ( std::string const& abs_path_to_module
     , std::string const& method
     , boost::python::list const& locations_and_parameters
     );

  /**
  * Executes a job on the workers asynchronously.
  *
  * For a description of the method see drts_wrapper::run. Contrary to synchronous execution
  * the return value of the asynchronous execution is only an id which can be used
  * to later retrieve the results.
  *
  * @return the id of the executed job.
  */
  gspc::job_id_t async_run
     ( std::string const& abs_path_to_module
     , std::string const& method
     , boost::python::list const& locations_and_parameters
     , std::string const& output_directory
     );

  /**
  * Collects the results of an executed job.
  *
  * Blocking call to retrieve all the results of an executed job.
  *
  * @param job_id the id of the job
  * @return       a list with all the generated results
  */
  boost::python::list collect_results
    (gspc::job_id_t const& job_id);

  /**
  * Checks the number of all remaining tasks in a job.
  *
  * If the number of remaining tasks is zero, the job is completed.
  *
  * @param job the id of the job
  * @return    the number of remaining tasks
  */
  unsigned long get_number_of_remaining_tasks
    (gspc::job_id_t const& job);

  /**
  * Retrieves the number of all tasks of the job.
  *
  * The number of all tasks that a job has to complete before it is finished.
  *
  * @param job the id of the job
  * @return    the number of all tasks
  */
  unsigned long get_total_number_of_tasks
     (gspc::job_id_t const& job);

  /**
  * Retrieves a result of a job.
  *
  * Blocking call to collect a result (from the worker that finished next) of 
  * a job.
  *
  * @param job the id of the job
  * @return    the next result
  */
  boost::python::object pop_result (gspc::job_id_t const& job);

  /**
  * Checks whether a new result is available.
  *
  * @return true if a new result is available
  */
  bool is_result_available(gspc::job_id_t const& job);

  /**
  * Stops the runtime.
  */
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

  pnetc::type::task_result::task_result get_next_task_result
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
  std::unique_ptr<result_storage> _result_storage;
  boost::uuids::random_generator _generator;
};
