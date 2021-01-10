#include "gspc_interface.hpp"

#include <iostream>
#include <unordered_set>

#include <boost/asio/ip/host_name.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>

#include <pnetc/type/config/op.hpp>
#include <pnetc/type/task_result/op.hpp>
#include <pnetc/type/location_and_parameters.hpp>
#include <pnetc/type/location_and_parameters/op.hpp>

#include <drts/virtual_memory.hpp>
#include <drts/information_to_reattach.hpp>
#include <drts/worker_description.hpp>

#include <we/type/value.hpp>
#include <we/type/value/path/join.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/serialize.hpp>

#include "log/log.hpp"

namespace
{
  boost::program_options::variables_map create_vm(const installation& install, const boost::property_tree::ptree& config)
  {
    boost::program_options::variables_map vm;

    boost::program_options::options_description desc("GSPC");

    desc.add(gspc::options::drts());
    desc.add(gspc::options::logging());
    desc.add(gspc::options::scoped_rifd());

    std::vector<std::string> arr_opt;

    arr_opt.push_back("--nodefile");
    arr_opt.push_back("");

    arr_opt.push_back("--worker-env-copy-current");

    std::string rif_strategy = config.get("gspc.rif_strategy", "ssh");
    arr_opt.push_back("--rif-strategy");
    arr_opt.push_back(rif_strategy);

    if (config.get("gspc.ssh_private_key", "") != "")
    {
      std::string ssh_private_key("--ssh-private-key=" + config.get("gspc.ssh_private_key", ""));

      arr_opt.push_back("--rif-strategy-parameters");
      arr_opt.push_back(ssh_private_key);
    }

    if (config.get("gspc.ssh_public_key", "") != "")
    {
      std::string ssh_public_key("--ssh-public-key=" + config.get("gspc.ssh_public_key", ""));
      
      arr_opt.push_back("--rif-strategy-parameters");
      arr_opt.push_back(ssh_public_key);
    }

    if (config.get("gspc.ssh_username", "") != "")
    {
      std::string ssh_username("--ssh-username=" + config.get("gspc.ssh_username", ""));

      arr_opt.push_back("--rif-strategy-parameters");
      arr_opt.push_back(ssh_username);
    }

    if (config.get("gspc.ssh_port", "") != "")
    {
      std::string ssh_username("--ssh-port=" + config.get("gspc.ssh_port", ""));

      arr_opt.push_back("--rif-strategy-parameters");
      arr_opt.push_back(ssh_username);
    }

    boost::program_options::store
    (boost::program_options::command_line_parser(arr_opt).options(desc).run(), vm);

    boost::program_options::notify(vm);

    gspc::set_gspc_home(vm, install.gspc());
    gspc::set_application_search_path(vm, install.libraries());

    return vm;
  }
}

gspc_interface::gspc_interface(const installation& install, const boost::property_tree::ptree& config)
: _vm(create_vm(install, config))
, _workflow(install.workflow())
, _stop_workers(install.stop_workers_script())
, _installation(_vm)
, _certificates(config.get("gspc.certificates_dir", "") != ""
  ? gspc::Certificates(config.get("gspc.certificates_dir", ""))
  : boost::none)
{
  _master_rifd = std::make_unique<gspc::scoped_rifd>(
      gspc::rifd::strategy(_vm)
    , gspc::rifd::hostname(boost::asio::ip::host_name())
    , gspc::rifd::port(_vm)
    , _installation
    );

  _drts = std::make_unique<gspc::scoped_runtime_system>(
        _vm
      , _installation
      , ""
      , boost::none
      , _master_rifd->entry_point()
      , std::cerr
      , _certificates
      );

  auto information_to_reattach = gspc::information_to_reattach(*_drts);
  auto string = information_to_reattach.to_string();
  pnet::type::value::value_type deserialized = pnet::type::value::from_string(string);
  {
    auto v = pnet::type::value::peek(std::list<std::string>{ "top_level_agent", "host" }, deserialized);
    if (v)
      _agent_host = boost::get<std::string>(*v);
  }
  {
    auto v = pnet::type::value::peek(std::list<std::string>{ "top_level_agent", "port" }, deserialized);
    if (v)
      _agent_port = static_cast<unsigned short>(boost::get<unsigned int>(*v));
  }

  log_message::info("[gspc_interface::gspc_interface] started agent on host '" + _agent_host
    + "' on port '" + std::to_string(_agent_port) + "'");
}

gspc_interface::~gspc_interface()
{
  _drts.reset();
  _master_rifd.reset();
}

void gspc_interface::add_workers(const std::vector<std::string>& hosts, unsigned workers_per_host,
  const std::vector<std::string>& capabilities, unsigned shm_size)
{
  log_message::info("[gspc_interface::add_workers] starting rifd");
  
  // Starting rifds on remote workers
  auto rifds = gspc::rifds(
      gspc::rifd::strategy(_vm)
    , gspc::rifd::port(_vm)
    , _installation
    );

  // and bootstrapping them
  log_message::info("[gspc_interface::add_workers] bootstrapping rifds");
  auto [entry_points, errors] = rifds.bootstrap(gspc::rifd::hostnames(hosts));
  // TODO : Check for errors

  const gspc::worker_description description
  {
      capabilities
    , workers_per_host
    , 0
    , shm_size
    , boost::none
    , boost::none
   };

  // Adding the worker
  log_message::info("[gspc_interface::add_workers] adding workers");
  _drts->add_worker({ description }, entry_points, _certificates);

  // TODO: Add tokens on workflow for each worker to report back!

  // and finally tear down the rifds.
  log_message::info("[gspc_interface::add_workers] teardown rifds");
  rifds.teardown();
}
  
void gspc_interface::remove_workers(const std::vector<std::string>& hosts)
{
  // Starting rifds on remote workers
  log_message::info("[gspc_interface::remove_workers] starting rifds");
  auto rifds = gspc::rifds(
    gspc::rifd::strategy(_vm)
    , gspc::rifd::port(_vm)
    , _installation
  );

  // and bootstrapping them
  log_message::info("[gspc_interface::remove_workers] bootstrapping rifds");
  auto [entry_points, errors] = rifds.bootstrap(gspc::rifd::hostnames(hosts));
  // TODO : Check for errors
  
  // Removing workers
  log_message::info("[gspc_interface::remove_workers] removing workers");
  //_drts->remove_worker(entry_points); // does not work
  std::unordered_set<std::string> hostnames;
  for (auto& host : hosts)
    hostnames.insert(host);

  std::pair< std::unordered_map<std::string, std::vector<std::string>>
    , std::unordered_map<std::string, std::exception_ptr>
  > a = rifds.execute(hostnames, _stop_workers);

  std::string message;
  for (auto& it : a.first)
  {
    message += it.first + "(";

    for (auto& iter : it.second)
      message += iter + ", ";
    message += ")";
  }

  for (auto& it : a.second)
  {
    message += it.first + "(";
    try
    {
      std::rethrow_exception(it.second);
    }
    catch (std::exception & exc)
    {
      message += exc.what();
    }
    message += ")";
  }
  log_message::info(message);
  // Marking workers as unavailable

  // and finally removing the rifds
  log_message::info("[gspc_interface::remove_workers] teardown rifds");
  rifds.teardown();
}

boost::optional<void*> gspc_interface::fetch_available_worker_info()
{
  return boost::none;
}

std::vector<result> gspc_interface::fetch_available_results()
{
  std::vector<result> results;
  decltype(_jobs) jobs;
  {
    std::lock_guard<std::mutex> guard(_jobs_mutex);
    jobs = _jobs; // Copy jobs
  }

  for (auto iter = jobs.begin(); iter != jobs.end(); ++iter)
  {
    auto job = iter->second;
    // Check remaining tasks:
    gspc::client client(*_drts, _certificates);
    unsigned n_left_tasks = 0;
    try
    {
      n_left_tasks = boost::get<unsigned long>(
        client.synchronous_workflow_response(job, "get_number_of_remaining_tasks", 0UL));
    }
    catch (std::runtime_error & err)
    {
      // Workflow appears to be finished
      log_message::info("[gspc_interface::get_last_result] " + std::string(err.what()));
      try
      {
        auto const res(client.extract_result_and_forget_job(job));
        auto range = res.equal_range("task_result");

        for (auto it = range.first; it != range.second; ++it)
        {
          auto task_result = pnetc::type::task_result::from_value(it->second);

          results.push_back(result{
              iter->first,
              task_result.worker,
              task_result.location,
              task_result.start_time,
              task_result.duration,
              task_result.error,
              task_result.success.to_string()
            });
        }
      }
      catch (std::runtime_error & err2)
      {
        log_message::info("[gspc_interface::get_last_result] " + std::string(err2.what()));
      }      
    }

    unsigned last_remaining_tasks = 0;
    {
      std::lock_guard<std::mutex> guard(_remaining_tasks_mutex);
      auto iter = _remaining_tasks.find(job);
      if (iter != _remaining_tasks.end())
        last_remaining_tasks = iter->second;
    }

    // There are results available
    for (auto i = n_left_tasks; i < last_remaining_tasks; ++i)
    {
      try
      {
        pnetc::type::task_result::task_result task_result;
        pnet::type::value::value_type response(client.synchronous_workflow_response
          (job, "get_next_task_result", pnetc::type::task_result::to_value(task_result)));
        task_result = pnetc::type::task_result::from_value(response);
        
        results.push_back(result{
            iter->first,
            task_result.worker,
            task_result.location,
            task_result.start_time,
            task_result.duration,
            task_result.error,
            task_result.success.to_string()
          });
      }
      catch (std::runtime_error & err)
      {
        // Workflow appears to be finished
        log_message::info("[gspc_interface::get_last_result] " + std::string(err.what()));
      }
    }

    if(n_left_tasks > 0)
    {
      std::lock_guard<std::mutex> guard(_remaining_tasks_mutex);
      auto iter = _remaining_tasks.find(job);
      if (iter != _remaining_tasks.end())
        iter->second = n_left_tasks;
    }
    else
    {
      std::lock_guard<std::mutex> guard(_remaining_tasks_mutex);
      std::lock_guard<std::mutex> guard2(_jobs_mutex);

      _remaining_tasks.erase(job);
      auto [start, end] = _jobs.equal_range(iter->first);
      for (auto it = start; it != end; ++it)
      {
        if (it->second == job)
        {
          _jobs.erase(it);
          break;
        }
      }
    }
  }
  return results;
}

boost::optional<result> gspc_interface::get_last_result(const std::string& worker)
{
  (void)worker;
  return boost::none;
}

void gspc_interface::start_job(const std::string& job_name, const job_config& config,
  const std::list<std::pair<std::string, std::string>>& location_and_parameters)
{
  std::list<pnet::type::value::value_type> location_and_parameters_list;
  for (auto& loc_para : location_and_parameters)
  {
    // if(known_location(loc_para)
    location_and_parameters_list.emplace_back(
      pnetc::type::location_and_parameters::to_value(
        pnetc::type::location_and_parameters::location_and_parameters
        ("", loc_para.first, loc_para.second)));
  }

  log_message::info("[gspc_interface::start_job] created list");
  pnetc::type::config::config c(
    config.path_to_module_or_module_content
    , config.is_path ? "true" : "false"
    , config.method
  );
  log_message::info(std::string("[gspc_interface::start_job] config = ")
  //  + "\n" + c.python_home
    + "\n" + c.path_to_module_or_module_content
    + "\n" + c.is_path
    + "\n" + c.method
  //  + "\n" + c.output_directory
  );
  log_message::info("[gspc_interface::start_job] created config");

  auto client = gspc::client(*_drts, _certificates);
  log_message::info("[gspc_interface::start_job] created client");
  auto wf = gspc::workflow(_workflow);
  log_message::info("[gspc_interface::start_job] created wf");
  auto config_pnetc = pnetc::type::config::to_value(c);
  log_message::info("[gspc_interface::start_job] config as pnetc");
  auto job_id = client.submit(wf,
    { {"config", config_pnetc},
      {"parameter_list", location_and_parameters_list},
      {"n_tasks", location_and_parameters_list.size()}
    }
  );

  log_message::info("[gspc_interface::start_job] submitted job");

  std::lock_guard<std::mutex> guard(_jobs_mutex);
  std::lock_guard<std::mutex> guard2(_remaining_tasks_mutex);
  _jobs.insert({ job_name, job_id });
  _remaining_tasks.insert({ job_id, location_and_parameters_list.size() });
}

void gspc_interface::cancel_job(const std::string& job_name)
{
  std::lock_guard<std::mutex> guard(_jobs_mutex);
  std::lock_guard<std::mutex> guard2(_remaining_tasks_mutex);
  auto [iter, end] = _jobs.equal_range(job_name);
  
  for (; iter != end; ++iter)
  {
    try
    {
      gspc::client(*_drts, _certificates).cancel(iter->second);
      _remaining_tasks.erase(iter->second);
    }
    catch (const std::exception& exception)
    {
      log_message::info("[gspc_interface::cancel_job] " + std::string(exception.what()));
    }
  }
}