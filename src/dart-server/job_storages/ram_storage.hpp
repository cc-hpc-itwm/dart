#pragma once

#include "dart-server/job_storage.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <unordered_map>

namespace job_storages
{
  class ram final : public job_storage
  {
  public:
    virtual ~ram() = default;
    
    /**
    * Adds a job to the storage.
    *
    * @param job        the name of the job
    * @param job_config the configuration of the job
    */
    virtual void add_job(const std::string& job, const job_config& job_config) override;

    /**
    * Gets information about a job.
    *
    * @param  the name of the job
    * @return the state and config of the job
    */
    virtual std::pair<job_status, job_config> get_job(const std::string& name) const override;

    /**
    * Gets all jobs.
    *
    * Returns a list of all jobs with all information (name, status, config)
    * for synchronization purposes.
    *
    * @return a list of all jobs
    */
    virtual std::vector<std::tuple<std::string, job_status, job_config>>
      get_jobs() const override;

    /**
    * Gets the status of a job
    *
    * @param job the job name
    * @return    the state of the job
    */
    virtual job_status get_status(const std::string& job) const override;

    /**
    * Sets the status of a job
    *
    * @param job    the job name
    * @param status the status
    */
    virtual void set_status(const std::string& job, job_status status) override;

    /**
    * Adds a result.
    *
    * @param result the result to add
    */
    virtual void add_result(const result& result) override;

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
      get_results(const std::string& job, unsigned amount, const std::string& worker_regex) const override;


    /**
    * Deletes a result.
    *
    * @param job    the job name
    * @param result the result id
    */
    virtual void delete_result(const std::string& job, const std::string& result) override;
  private:
    std::unordered_map<std::string, std::pair<job_status, job_config>> _jobs; // job -> (status, config) map
    std::unordered_map<std::string, std::vector<std::pair<std::string, result>>> _results; // job -> (id, result) list

    boost::uuids::basic_random_generator<boost::mt19937> _generator;
  };
}