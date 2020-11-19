#pragma once

#include <cpp/task_result.hpp>
#include <drts/client.hpp>
#include <boost/optional.hpp>

/**
* Abstract class for storing results temporarily.
*
* Depending on the configuration different result storages can be used,
* e.g., ram storage, file storage
*
* The result storage operators like a FIFO queue, e.g., the first result
* that has been put into the storage for a given job will be the result
* one gets.
*/
class result_storage : boost::noncopyable
{
public:
  using optional_result = boost::optional<task_result>;
public:
  result_storage() = default;
  virtual ~result_storage() = default;

  /**
  * Adds a job to the storage.
  *
  * Adds a job to the storage and saves number of total expected results.
  *
  * @param job_id           the id of the job
  * @param expected_results the number of expected results for this job
  */
  virtual void add_job(const gspc::job_id_t& job_id, unsigned expected_results) = 0;

  /**
  * Removes a job.
  *
  * Removes a job from storage and frees all stored results.
  *
  * @param job_id the id of the job
  */
  virtual void remove_job(const gspc::job_id_t& job_id) = 0;

  /**
  * Checks whether all expected results have been received.
  *
  * @return true, if at least the amount of expected reults have been received
  */
  inline bool have_expected_results_been_received(const gspc::job_id_t& job_id) const
  {
    return (static_cast<long long>(get_number_of_expected_results(job_id))
      - get_number_of_received_results(job_id)) <= 0;
  }

  /**
  * Returns the number of the expected results.
  *
  * @param job_id the job_id to lookup the expected results for
  * @return       the number of expected results
  */
  virtual unsigned get_number_of_expected_results(const gspc::job_id_t& job_id) const = 0;

  /**
  * Returns the number of the received results.
  *
  * @param job_id the job_id to lookup the received results for
  * @return       the number of received results
  */
  virtual unsigned get_number_of_received_results(const gspc::job_id_t& job_id) const = 0;

  /**
  * Adds a result to the storage.
  *
  * @param job_id      the id of the job to be added to the storage
  * @param task_result the result of the executed task
  */
  virtual void push(const gspc::job_id_t& job_id, task_result&& task_result) = 0;

  /**
  * Gets a result if available.
  *
  * This function does not remove the result from the storage,
  * i.e., executing get multiple times without calling 
  * result::storage.::pop will always yield the same result.
  *
  * @param job_id the id of the job to get the result from
  * @return       the result, if there is one
  */
  virtual optional_result get(const gspc::job_id_t& job_id) const = 0;

  /**
  * Removes the earliest result from the storage.
  *
  * @param job_id the id of the job to remove the result from
  */
  virtual void pop(const gspc::job_id_t& job_id) = 0;
};