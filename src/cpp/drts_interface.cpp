#include <cpp/drts_interface.hpp>

#include <cpp/result_storages/ram_storage.hpp>
#include <cpp/result_storages/python_storage.hpp>
#include <cpp/task_result.hpp>

#include <utils/utils.hpp>

#include <pnetc/type/task_result/op.hpp>

#include <pnetc/type/config/op.hpp>
#include <pnetc/type/location_and_parameters.hpp>
#include <pnetc/type/location_and_parameters/op.hpp>

#include <drts/client.hpp>
#include <drts/virtual_memory.hpp>
#include <drts/worker_description.hpp>

#include <we/type/value/from_value.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/unwrap.hpp>
#include <we/type/value/wrap.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/init.hpp>
#include <boost/python/exception_translator.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include "boost/program_options.hpp"
#include "boost/filesystem.hpp"
#include "boost/any.hpp"
#include "boost/iostreams/stream.hpp"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <future>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <thread>
#include <dlfcn.h>

namespace po = boost::program_options;
namespace 
{
      Dl_info dladdr2 (void* addr)
      {
        Dl_info info;
        dladdr (addr, &info);
        return info;
      }
}

drts_wrapper::drts_wrapper
    ( std::string const& python_home
    , boost::python::dict const& user_opt
    )
  : _vm (get_options (user_opt))
  , _certificates
      ( user_opt.has_key ("certificates_dir")
      ? gspc::Certificates (boost::python::extract<std::string> (user_opt["certificates_dir"]))
      : boost::none
      )
  , _installation (boost::filesystem::canonical (dladdr2 (const_cast<char*>("drts_wrapper")).dli_fname).parent_path().parent_path())
  , _gspc_installation (_installation.gspc_installation (_vm))
  , _master_rifd (new gspc::scoped_rifd
                    ( gspc::rifd::strategy (_vm)
                    , gspc::rifd::hostname (boost::asio::ip::host_name())
                    , gspc::rifd::port (_vm)
                    , _gspc_installation
                    )
                 )
  , _drts (nullptr)
  , _python_home (python_home)
  , _result_storage()
{
  std::cout << dladdr2 (const_cast<char*>("drts_wrapper")).dli_fname << std::endl;
  std::cout << boost::filesystem::canonical (dladdr2 (const_cast<char*>("drts_wrapper")).dli_fname).parent_path().parent_path() << std::endl;

  if (!user_opt.has_key("result_storage"))
    _result_storage = std::make_unique<result_storages::ram_storage>();
  else
    _result_storage = std::make_unique<result_storages::python_storage>(user_opt["result_storage"]);
}

void drts_wrapper::start_runtime()
{

  _drts = std::make_unique<gspc::scoped_runtime_system>
            ( _vm
            , _gspc_installation
            , ""
            , boost::none
            , _master_rifd->entry_point()
            , std::cerr
            , _certificates
            );
}

void drts_wrapper::start_runtime (std::string const& nodefile)
{
  start_runtime();
  add_workers (nodefile);
}

void drts_wrapper::start_runtime
  ( std::string const& nodefile
  , std::size_t num_workers
  )
{
  start_runtime();
  boost::python::list capabilities;
  capabilities.append ("local_cluster");
  add_workers (nodefile, num_workers, capabilities, 0);
}

namespace 
{
    [[maybe_unused]]
    void
    print_variable_map (boost::program_options::variables_map &vm)
    {
        for (const auto& it : vm) 
        {
            std::cout << it.first.c_str() << " ";
            auto& value = it.second.value();
            if (auto v = boost::any_cast<uint32_t> (&value))
                std::cout << "\t" << *v << std::endl;
            else if (auto v = boost::any_cast<std::string> (&value))
                std::cout << "\t" << *v << std::endl;
            else if (auto v = boost::any_cast<std::vector<std::string>> (&value))
            for (auto s : *v)
                std::cout << "\t" << s << std::endl;
            else
                std::cout << "\t" << "error" << std::endl;
        }
    }
}

descriptions_and_entry_points_t drts_wrapper::parse_worker_description_file
  (boost::filesystem::path const& nodefile)
{
  boost::property_tree::ptree pt;

  boost::property_tree::json_parser::read_json
    (boost::filesystem::canonical (nodefile).string(), pt);

  descriptions_and_entry_points_t descriptions_and_entry_points;

  for (boost::property_tree::ptree::value_type const& e : pt.get_child(""))
  {
    std::vector<std::string> capabilities;
    for (auto const& v : e.second.get_child ("capabilities"))
    {
      capabilities.push_back (v.second.data());
    }

    auto const num_per_node (e.second.get<std::size_t> ("num_per_node"));
    auto const shm_size (e.second.get<unsigned long> ("shm_size"));
    auto const port (e.second.get<std::size_t> ("port"));

    std::vector<std::string> hosts;
    for (auto const& v : e.second.get_child ("hosts"))
    {
      hosts.push_back (v.second.data());
    }

    boost::program_options::variables_map vm; 

    std::vector<std::string> new_paras;
    for (const auto& it : _vm) 
    {
        std::cout << it.first.c_str() << " ";
        if (it.first == "rif-strategy-parameters")
        { 
            auto& value = it.second.value();
            auto v = boost::any_cast<std::vector<std::string>> (&value);
            for (auto s : *v)
            {
                if (s.find ("--ssh-port") != std::string::npos)
                {
                    new_paras.emplace_back ("--ssh-port=" + std::to_string (port));
                }
                else
                {
                    new_paras.emplace_back (s);
                }
            }
        }
    }


    vm.emplace (std::string ("rif-strategy-parameters")
               , boost::program_options::variable_value
                  (new_paras, false)
               );

    vm.emplace (std::string ("rif-strategy")
               , boost::program_options::variable_value
                  (std::string {"ssh"}, false)
               );


    _worker_rifds.emplace_back
      (new gspc::scoped_rifds ( gspc::rifd::strategy (vm) // here we need the ssh port
                              , gspc::rifd::hostnames (hosts)
                              , gspc::rifd::port (_vm) // here we need the rifd port
                              , _gspc_installation
                              )
      );

    gspc::worker_description const description
      { capabilities
      , num_per_node
      , 0
      , shm_size
      , boost::none
      , boost::none
      };

    descriptions_and_entry_points.emplace_front
      ( std::make_pair ( description
                       , (*_worker_rifds.rbegin())->entry_points()
                       )
      );
  }

  return descriptions_and_entry_points;
}

void drts_wrapper::add_workers
  (descriptions_and_entry_points_t const& descriptions_and_entry_points)
{
  for (auto const& description : descriptions_and_entry_points)
  {
    _drts->add_worker
      ({description.first}, description.second, _certificates);

    _descriptions_and_entry_points.emplace_front (description);
  }
}

void drts_wrapper::add_workers (std::string const& nodefile)
{
  add_workers (parse_worker_description_file (nodefile));
}


void drts_wrapper::add_workers
  ( std::string const& machinefile
  , std::size_t workers_per_host
  , boost::python::list const& capabilities
  , std::size_t shm_size
  )
{
  if (!boost::filesystem::exists (machinefile))
  {
    throw std::runtime_error
      ("The machine file \"" + machinefile + "\" does not exist!");
  }

  std::ifstream ifs (machinefile);

  std::vector<std::string> hosts;
  std::copy ( std::istream_iterator<std::string> (ifs)
            , std::istream_iterator<std::string>()
            , std::back_inserter (hosts)
            );

  _worker_rifds.emplace_back
    (new gspc::scoped_rifds ( gspc::rifd::strategy (_vm)
                            , gspc::rifd::hostnames (hosts)
                            , gspc::rifd::port (_vm)
                            , _gspc_installation
                            )
    );

  gspc::worker_description const description
    { dart::to_std_vector<std::string> (capabilities)
    , workers_per_host
    , 0
    , shm_size
    , boost::none
    , boost::none
    };

  add_workers
    ({std::make_pair (description, (*_worker_rifds.rbegin())->entry_points())});
}

void drts_wrapper::add_workers
  ( boost::python::list const& pyhosts
  , std::size_t workers_per_host
  , boost::python::list const& capabilities
  , std::size_t shm_size
  )
{
  _worker_rifds.emplace_back
     (new gspc::scoped_rifds ( gspc::rifd::strategy (_vm)
                             , gspc::rifd::hostnames ( dart::to_std_vector<std::string> (pyhosts))
                             , gspc::rifd::port (_vm)
                             , _gspc_installation
                             )
     );

   gspc::worker_description const description
     { dart::to_std_vector<std::string> (capabilities)
     , workers_per_host
     , 0
     , shm_size
     , boost::none
     , boost::none
     };

   add_workers
     ({std::make_pair (description, (*_worker_rifds.rbegin())->entry_points())});
}

std::list<pnet::type::value::value_type> drts_wrapper::get_location_and_parameters
  (boost::python::list const& locations_and_parameters)
{
  std::list<pnet::type::value::value_type> required_location_and_parameters;

  boost::python::ssize_t const num_locations_and_params
    {len (locations_and_parameters)};

  for (auto i {0}; i < num_locations_and_params; ++i)
  {
    boost::python::dict const dict_loc_params
      {locations_and_parameters[i]};

    std::string location;
    if (dict_loc_params.has_key ("location"))
    {
      location = {boost::python::extract<std::string> (dict_loc_params["location"])};
      if (!known_location (location))
      {
        stop_runtime();
        throw std::runtime_error
          ("No worker is running at location " + location +"!");
      }
    }

    if (!dict_loc_params.has_key ("parameters"))
    {
      throw std::runtime_error
        ("No key \'parameters\' is specified in the submitted list!");
    }

    boost::python::list const parameters {dict_loc_params["parameters"]};
    boost::python::ssize_t const num_parameter_sets {(len (parameters))};

    for (auto i {0}; i < num_parameter_sets; ++i)
    {
      std::string parameter_set {boost::python::extract<std::string> (parameters[i])};

      boost::uuids::uuid uuid = _generator();
      pnetc::type::location_and_parameters::location_and_parameters loc_and_params
        (boost::uuids::to_string (uuid), location, parameter_set);

      required_location_and_parameters.emplace_back
        (pnetc::type::location_and_parameters::to_value (loc_and_params));
    }
  }

  return required_location_and_parameters;
}

unsigned long drts_wrapper::get_number_of_tasks
  (boost::python::list const& locations_and_parameters)
{
  unsigned long n_tasks = 0;

  boost::python::ssize_t const num_locations_and_params
     {len (locations_and_parameters)};

  for (auto i {0}; i < num_locations_and_params; ++i)
  {
    boost::python::dict const dict_loc_params (locations_and_parameters[i]);

    if (dict_loc_params.has_key ("location"))
    {
      std::string const location
        {boost::python::extract<std::string> (dict_loc_params["location"])};

      if (!known_location (location))
      {
        stop_runtime();
        throw std::runtime_error
          ("No worker is running at location " + location +"!");
      }
    }

    if (!dict_loc_params.has_key("parameters"))
    {
      throw std::runtime_error
        ("No key \'parameters\' is specified in the submitted list!");
    }
    n_tasks += len (dict_loc_params["parameters"]);
  }

  return n_tasks;
}

boost::python::list drts_wrapper::run
  ( std::string const& method
  , boost::python::list const& locations_and_parameters
  )
{
  return run (method, locations_and_parameters, {});
}

boost::python::list drts_wrapper::run
  ( std::string const& method
  , boost::python::list const& locations_and_parameters
  , std::string const& output_directory
  )
{
  return run
    ( (_installation.home() / "lib/dart_task.py").string()
    , method
    , locations_and_parameters
    , output_directory
    );
}

boost::python::list drts_wrapper::run
  ( std::string const& abs_path_to_module
  , std::string const& method
  , boost::python::list const& locations_and_parameters
   )
{
  return run
    (abs_path_to_module, method, locations_and_parameters, {});
}

boost::python::list drts_wrapper::run
  ( std::string const& abs_path_to_module
  , std::string const& method
  , boost::python::list const& locations_and_parameters
  , std::string const& output_directory
  )
{
  boost::filesystem::path const path (abs_path_to_module);
  std::string const module (path.stem().string());

  unsigned long n_total_tasks (get_number_of_tasks (locations_and_parameters));

  pnetc::type::config::config config
    ( _python_home.string()
    , path.parent_path().string()
    , module
    , method
    , output_directory
    );

  std::multimap<std::string, pnet::type::value::value_type> const exec_res
    ( gspc::client (*_drts, _certificates).put_and_run
        ( gspc::workflow (_installation.workflow())
        , { {"config", pnetc::type::config::to_value (config)}
          , {"parameter_list", get_location_and_parameters (locations_and_parameters)}
          , {"n_tasks", n_total_tasks}
          }
        )
    );

  auto range(exec_res.equal_range("task_result"));
  boost::python::list results;
  for (auto it = range.first; it != range.second; ++it)
    results.append(task_result_to_python(pnetc::type::task_result::from_value(it->second)));
  return results;
}

gspc::job_id_t drts_wrapper::async_run
  ( std::string const& method
  , boost::python::list const& locations_and_parameters
  , std::string const& output_directory
  )
{
  return async_run
    ( (_installation.home() / "lib/dart_task.py").string()
    , method
    , locations_and_parameters
    , output_directory
    );
}

gspc::job_id_t drts_wrapper::async_run
  ( std::string const& method
  , boost::python::list const& locations_and_parameters
  )
{
  return async_run (method, locations_and_parameters, {});
}

gspc::job_id_t drts_wrapper::async_run
  ( std::string const& abs_path_to_module
  , std::string const& method
  , boost::python::list const& locations_and_parameters
  )
{
  return async_run
    (abs_path_to_module, method, locations_and_parameters, {});
}

gspc::job_id_t drts_wrapper::async_run
  ( std::string const& abs_path_to_module
  , std::string const& method
  , boost::python::list const& locations_and_parameters
  , std::string const& output_directory
  )
{
  boost::filesystem::path const path (abs_path_to_module);
  std::string const module (path.stem().string());
  unsigned long n_total_tasks (get_number_of_tasks (locations_and_parameters));

  pnetc::type::config::config config
    ( _python_home.string()
    , path.parent_path().string()
    , module
    , method
    , output_directory
    );

  gspc::job_id_t const job_id
    ( gspc::client (*_drts, _certificates).submit
        ( gspc::workflow (_installation.workflow())
        , { {"config", pnetc::type::config::to_value (config)}
          , {"parameter_list", get_location_and_parameters (locations_and_parameters)}
          , {"n_tasks", n_total_tasks}
          }
        )
     );

  _result_storage->add_job(job_id, n_total_tasks);

  return job_id;
}

boost::python::list drts_wrapper::collect_results
  (gspc::job_id_t const& job_id)
{
  gspc::client client (*_drts, _certificates);

  client.wait (job_id);

  auto exec_res(client.extract_result_and_forget_job(job_id));

  auto range(exec_res.equal_range("task_result"));
  boost::python::list results;
  for (auto it = range.first; it != range.second; ++it)
    results.append(task_result_to_python(pnetc::type::task_result::from_value(it->second)));

  auto result = _result_storage->get(job_id);
  while (result)
  {
    _result_storage->pop(job_id);
    results.append(task_result_to_python(result.get()));
    result = _result_storage->get(job_id);
  }
  _result_storage->remove_job(job_id);
  return results;
}

boost::python::dict drts_wrapper::remove_workers()
{
  boost::python::dict errors;

  for (auto const& rifds : _worker_rifds)
  {
    gspc::rifd_entry_points entry_points (rifds->entry_points());

    std::unordered_map
      < gspc::rifd_entry_point
      , std::pair< std::string
                 , std::unordered_map<pid_t, std::exception_ptr>
                 >
      , gspc::rifd_entry_point_hash
      > const _warnings (_drts->remove_worker (entry_points));

    for (auto const& w : _warnings)
    {
      for (auto const& fail : w.second.second)
      {
        std::string excp;
        try 
        {
          if (fail.second) 
          {
            std::rethrow_exception (fail.second);
          }
        } catch(const std::exception& e) 
        {
          excp = e.what();
        }

        errors[w.first.hostname()]
          = ( boost::format ("%1%[%2%]: %3%")
            % w.second.first
            % fail.first
            % excp
            ).str();
      }
    }

    auto const result (rifds->teardown());

    for (std::string const& hostname : rifds->hosts())
    {
      if (result.second.count (hostname))
      {
        errors[hostname] = result.second.at (hostname);
      }
    }
  }

  return errors;
}

boost::python::dict drts_wrapper::remove_workers
  (boost::python::list const& py_hosts)
{
  return remove_workers
    (dart::to_std_vector<std::string> (py_hosts));
}

boost::python::dict drts_wrapper::remove_workers
  (std::vector<std::string> const& hosts)
{
  boost::python::dict errors;

  for (auto const& rifds : _worker_rifds)
  {
    std::pair< gspc::rifd_entry_points
             , std::pair< std::unordered_set<std::string>
                        , std::unordered_set<std::string>
                        >
             > const entry_points (rifds->entry_points (hosts));

    std::unordered_set<std::string> const& registered_hosts
      (entry_points.second.first);

    for (std::string const& host : entry_points.second.second)
    {
      errors[host] = "unknown host";
    }

    std::unordered_map
      < gspc::rifd_entry_point
      , std::pair< std::string
                 , std::unordered_map<pid_t, std::exception_ptr>
                 >
      , gspc::rifd_entry_point_hash
      > const _warnings (_drts->remove_worker (entry_points.first));

    for (auto const& w : _warnings)
    {
      for (auto const& fail : w.second.second)
      {

        std::string excp;
        try 
        {
          if (fail.second) 
          {
            std::rethrow_exception (fail.second);
          }
        } catch(const std::exception& e) 
        {
          excp = e.what();
        }
        errors[w.first.hostname()]
          = ( boost::format ("%1%[%2%]: %3%")
            % w.second.first
            % fail.first
            % excp
            ).str();
      }
    }

    auto const result (rifds->teardown (registered_hosts));

    for (std::string const& hostname : registered_hosts)
    {
      if (result.second.count (hostname))
      {
        errors[hostname] = result.second.at (hostname);
      }
    }
  }

  return errors;
}

unsigned long drts_wrapper::get_number_of_remaining_tasks
  (gspc::job_id_t const& job)
{
  gspc::client client (*_drts, _certificates);

  unsigned long n_left_tasks (0);

  try
  {
    n_left_tasks = boost::get<unsigned long>
      (client.synchronous_workflow_response
         (job, "get_number_of_remaining_tasks", 0UL)
      );
  }
  catch (...)
  {
    //ignore, the workflow finished in between!
  }

  return n_left_tasks;
}

unsigned long drts_wrapper::get_total_number_of_tasks
  (gspc::job_id_t const& job)
{
  return _result_storage->get_number_of_expected_results(job);
}

pnetc::type::task_result::task_result drts_wrapper::get_next_task_result
(gspc::client& client, gspc::job_id_t const& job)
{
  pnetc::type::task_result::task_result result;

  pnet::type::value::value_type response
    (client.synchronous_workflow_response
  (job, "get_next_task_result", pnetc::type::task_result::to_value(result))
  );

  return pnetc::type::task_result::from_value(response);
}

boost::python::object drts_wrapper::pop_result (gspc::job_id_t const& job)
{
  gspc::client client (*_drts, _certificates);

  try
  {
    _result_storage->push(job, get_next_task_result (client, job));
  }
  catch (...) // the workflow has finished in between
  {
    try
    {
      auto const results(client.extract_result_and_forget_job(job));
      auto range = results.equal_range("task_result");

      for (auto it = range.first; it != range.second; ++it)
      {
        _result_storage->push(job, pnetc::type::task_result::from_value(it->second));
      }
    }
    catch (...)
    {
      // Workflow has finished and all results have been extracted
    }
  }

  auto result = _result_storage->get(job);
  if (!result && _result_storage->have_expected_results_been_received(job))
  {
    throw std::logic_error
      ("No results are left. All results were retrieved!");
  }
  else if(result)
  {
    _result_storage->pop(job);
    return task_result_to_python(result.get());
  }
  throw std::logic_error("Waited for result, but no results available!");
}


/**
* Checks whether a new result is available.
*
* @return true if a new result is available
*/
bool drts_wrapper::is_result_available(gspc::job_id_t const& job)
{
  auto remaining(get_number_of_remaining_tasks(job));
  auto received(_result_storage->get_number_of_received_results(job));
  auto expected(_result_storage->get_number_of_expected_results(job));
  if (remaining + received < expected)
    return true;

  return false;
}

void drts_wrapper::stop_runtime()
{
  remove_workers();
  _drts.reset();
  _master_rifd.reset();
}

std::string extract_string (boost::python::object const& obj)
{
  char const* c_str = boost::python::extract<char const*> (obj);
  int size = boost::python::extract<int> (obj.attr("__len__")());
  return std::string (c_str, size);
}

int extract_int (boost::python::object const& obj)
{
  return boost::python::extract<int> (obj.attr("__int__")());
}

bool is_number(const std::string &s) 
{
  return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

boost::program_options::variables_map drts_wrapper::get_options
  (boost::python::dict const& user_params)
{
  namespace py = boost::python;

  boost::program_options::variables_map vm;

  boost::program_options::options_description desc ("GSPC");

  desc.add (gspc::options::drts());
  desc.add (gspc::options::logging());
  desc.add (gspc::options::scoped_rifd());

  std::vector<std::string> arr_opt;

  arr_opt.push_back ("--nodefile");
  arr_opt.push_back ("");

  arr_opt.push_back ("--worker-env-copy-current");

  std::string rif_strategy ( user_params.has_key ("rif_strategy")
                           ? extract_string (user_params["rif_strategy"])
                           : std::string ("ssh")
                           );
  arr_opt.push_back ("--rif-strategy");
  arr_opt.push_back (rif_strategy);

  if (user_params.has_key ("ssh_private_key"))
  {
    std::string dart_option
      (extract_string (user_params["ssh_private_key"]));

    std::string ssh_private_key
      ( ( boost::format ("--ssh-private-key=%1%")
        % dart_option
        ).str()
      );

    arr_opt.push_back ("--rif-strategy-parameters");
    arr_opt.push_back (ssh_private_key);
  }

  if (user_params.has_key ("ssh_public_key"))
  {
    std::string dart_option
      (extract_string (user_params["ssh_public_key"]));
    
    std::string ssh_public_key
      ( ( boost::format ("--ssh-public-key=%1%")
        % dart_option
        ).str()
      );

    arr_opt.push_back ("--rif-strategy-parameters");
    arr_opt.push_back (ssh_public_key);
  }

  if (user_params.has_key ("ssh_username"))
  {
    std::string dart_option
      (extract_string (user_params["ssh_username"]));

    std::string ssh_username
      ( ( boost::format ("--ssh-username=%1%")
        % dart_option
        ).str()
      );

    arr_opt.push_back ("--rif-strategy-parameters");
    arr_opt.push_back (ssh_username);
  }

  if (user_params.has_key ("ssh_port"))
  {
    int dart_option (extract_int (user_params["ssh_port"]));

    std::string ssh_port
      ( ( boost::format ("--ssh-port=%1%")
        % dart_option
        ).str()
      );

    arr_opt.push_back ("--rif-strategy-parameters");
    arr_opt.push_back (ssh_port);
  }

  boost::program_options::store
    (boost::program_options::command_line_parser (arr_opt).options (desc).run(), vm);

  boost::program_options::notify (vm);

  return vm;
}

bool drts_wrapper::known_location (std::string const& location)
{
  for (auto const& desc : _descriptions_and_entry_points)
  {
    if ( std::find ( desc.first.capabilities.begin()
                   , desc.first.capabilities.end()
                   , location
                   )
       !=  desc.first.capabilities.end()
       )
    { return true; }
  }

  return false;
}
