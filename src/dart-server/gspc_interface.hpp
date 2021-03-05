#pragma once

#include "dart-server/installation.hpp"
#include "dart-server/job_storage.hpp"

#include <mutex>
#include <string>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/optional/optional.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/property_tree/ptree.hpp>

#include <drts/drts.hpp>
#include <drts/client.hpp>
#include <drts/scoped_rifd.hpp>
#include <drts/certificates.hpp>

struct worker
{
  std::unordered_set<std::string> capabilities;
  unsigned count = 0;
};

struct ssh_options
{
  std::string username;
  boost::optional<unsigned short> port;
  std::string public_key;
  std::string private_key;
};

class gspc_interface final
{
public:
  gspc_interface(const installation& install, const boost::property_tree::ptree& config);
  ~gspc_interface();

  void add_workers(const std::string& name, const std::vector<std::string>& hosts, unsigned workers_per_host, 
    const std::vector<std::string>& capabilities, unsigned shm_size, const ssh_options& ssh);
  void remove_workers(const std::vector<std::string>& hosts, const ssh_options& ssh);

  std::unordered_map<std::string, worker> fetch_available_workers();

  std::vector<result> fetch_available_results();
  boost::optional<result> get_last_result(const std::string& worker);  // unimplemented
  
  void start_job(const std::string& job_name, const job_config& config, 
    const std::list<std::pair<std::string, std::string>>& location_and_parameters);
  void cancel_job(const std::string& job_name);

  inline auto& get_agent_host() const noexcept { return _agent_host; }
  inline auto& get_agent_port() const noexcept { return _agent_port; }

private:
  installation _dart_install;
  boost::program_options::variables_map _vm;
  boost::filesystem::path _workflow;
  boost::filesystem::path _stop_workers;
  gspc::installation _installation;
  gspc::Certificates _certificates;
  std::unique_ptr<gspc::scoped_rifd> _master_rifd;
  std::unique_ptr<gspc::scoped_runtime_system> _drts;

  std::string _agent_host;
  unsigned short _agent_port;

  std::mutex _jobs_mutex;
  std::unordered_multimap<std::string, gspc::job_id_t> 
    _jobs; // job_name -> job_ids
  std::mutex _remaining_tasks_mutex;
  std::unordered_map<gspc::job_id_t, unsigned> _remaining_tasks;
};