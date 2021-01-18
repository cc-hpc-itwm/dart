#pragma once

#include <string>
#include <vector>
#include <tuple>

enum class job_status
{
  unknown = 0,  // State of job is unknown
  running = 1,  // The job is running
  stopped = 2,  // The job has been stopped
};

struct job_config
{
  bool is_path;
  std::string path_to_module_or_module_content;
  std::string method;
};

struct result
{
  std::string job;
  std::string worker;
  std::string location;
  std::string start_time;
  float duration;
  std::string error;
  std::string success;
};

class job_storage
{
public:
  virtual ~job_storage() = default;

  /**
  * Checks whether there is a job with the given name.
  *
  * @param job the name of the job
  * @return    true, if there is a job with the given name
  */
  inline bool has_job(const std::string& job) const noexcept
  {
    return get_status(job) != job_status::unknown;
  }

  /**
  * Adds a job to the storage.
  *
  * @param job        the name of the job
  * @param job_config the configuration of the job
  */
  virtual void add_job(const std::string& job, const job_config& job_config) = 0;

  /**
  * Gets information about a job.
  *
  * @param  the name of the job
  * @return the state and config of the job
  */
  virtual std::pair<job_status, job_config> get_job(const std::string& name) const = 0;

  /**
  * Gets all jobs.
  *
  * Returns a list of all jobs with all information (name, status, config)
  * for synchronization purposes.
  *
  * @return a list of all jobs
  */
  virtual std::vector<std::tuple<std::string, job_status, job_config>>
    get_jobs() const = 0;

  /**
  * Gets the status of a job
  *
  * @param job the job name
  * @return    the state of the job
  */
  virtual job_status get_status(const std::string& job) const = 0;

  /**
  * Sets the status of a job
  *
  * @param job    the job name
  * @param status the status
  */
  virtual void set_status(const std::string& job, job_status status) = 0;


  /**
  * Adds a result.
  *
  * @param result the result to add
  */
  virtual void add_result(const result& result) = 0;

  /**
  * Returns a list of results.
  *
  * @param job          the job name
  * @param amount       the maximum amount of results to get
  * @param worker_regex only results whose worker match this regular expression will be returned
  *                     an empty regex should match every worker
  * @return             a list of id result pairs
  */
  virtual std::vector<std::pair<std::string, result>> 
    get_results(const std::string& job, unsigned amount, const std::string& worker_regex) const = 0;

  /**
  * Deletes a result.
  *
  * @param job    the job name
  * @param result the result id
  */
  virtual void delete_result(const std::string& job, const std::string& result) = 0;
};