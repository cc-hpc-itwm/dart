#include "dart-server/job_storages/ram_storage.hpp"

#include "log/log.hpp"

#include <boost/uuid/uuid_io.hpp>

#include <stdexcept>
#include <algorithm>

using namespace job_storages;

/**
* Adds a job to the storage.
*
* @param job        the name of the job
* @param job_config the configuration of the job
*/
void ram::add_job(const std::string& job, const job_config& job_config)
{
  log_message::info("Added job '" + job + "'");
  if (_jobs.find(job) != _jobs.end())
    throw std::runtime_error("[ram::add_job] job '" + job + "' already added");
  _jobs[job] = { job_status::running, job_config };
}

/**
* Gets information about a job.
*
* @param  the name of the job
* @return the state and config of the job
*/
std::pair<job_status, job_config> ram::get_job(const std::string& name) const
{
  log_message::info("Get job '" + name + "'");
  auto iter = _jobs.find(name);
  if (iter == _jobs.end())
    return { job_status::unknown, {} };
  else
    return iter->second;
}

/**
* Gets all jobs.
*
* Returns a list of all jobs with all information (name, status, config)
* for synchronization purposes.
*
* @return a list of all jobs
*/
std::vector<std::tuple<std::string, job_status, job_config>> ram::get_jobs() const
{
  log_message::info("Get jobs");
  std::vector<std::tuple<std::string, job_status, job_config>> jobs;
  jobs.reserve(_jobs.size());

  for (auto& job : _jobs)
    jobs.push_back({ job.first, job.second.first, job.second.second });

  return jobs;
}

/**
* Gets the status of a job
*
* @param job the job name
* @return    the state of the job
*/
job_status ram::get_status(const std::string& job) const
{
  log_message::info("Get job status '" + job + "'");
  auto iter = _jobs.find(job);
  if (iter == _jobs.end())
    return job_status::unknown;
  else
    return iter->second.first;
}

/**
* Sets the status of a job
*
* @param job    the job name
* @param status the status
*/
void ram::set_status(const std::string& job, job_status status)
{
  log_message::info("Set status '" + job + "'");
  if (_jobs.find(job) == _jobs.end())
    throw std::runtime_error("[ram::set_status] job '" + job + "' not available");
  _jobs[job].first = status;
}

/**
* Adds a result.
*
* @param result the result to add
*/
void ram::add_result(const result& result)
{
  log_message::info("Add result '" + result.job + "'");
  
  // TODO: what to do if _results[result.job] != running?
  auto id = boost::uuids::to_string(_generator());
  _results[result.job].push_back({ id, result });
}

/**
* Returns a list of results.
*
* @param job    the job name
* @param amount the maximum amount of results to get
* @return       a list of id result pairs
*/
std::vector<std::pair<std::string, result>> ram::get_results(const std::string& job, unsigned amount) const
{
  log_message::info("Get results");
  auto iter = _results.find(job);
  if (iter == _results.end())
    return {};

  std::vector<std::pair<std::string, result>> results;
  results.reserve(std::min(static_cast<size_t>(amount), iter->second.size()));
  for (auto i = 0u; i < std::min(static_cast<size_t>(amount), iter->second.size()); ++i)
  {
    results.push_back(iter->second[i]);
  }
  return results;
}

/**
* Deletes a result.
*
* @param job    the job name
* @param result the result id
*/
void ram::delete_result(const std::string& job, const std::string& result)
{
  log_message::info("Delete result '" + job + "'");
  auto iter = _results.find(job);
  if (iter == _results.end())
    return;

  for (auto it = iter->second.begin(); it != iter->second.end(); ++it)
  {
    if (it->first == result)
    {
      iter->second.erase(it);
      return;
    }
  }
}