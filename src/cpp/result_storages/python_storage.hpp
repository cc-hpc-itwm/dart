#pragma once

#include <pnetc/type/task_result.hpp>
#include <drts/client.hpp>
#include <boost/optional.hpp>
#include <boost/python.hpp>

#include "cpp/result_storage.hpp"

#include <unordered_map>
#include <queue>

namespace result_storages
{
  /**
  * Implementation of result_storage for objects that have been defined in python.
  */
  class python_storage final : public result_storage
  {
  public:
    python_storage(const boost::python::object& object);
    virtual ~python_storage() = default;

    /**
    * Adds a job to the storage.
    *
    * Adds a job to the storage and saves number of total expected results.
    *
    * @param job_id           the id of the job
    * @param expected_results the number of expected results for this job
    */
    virtual void add_job(const gspc::job_id_t& job_id, unsigned expected_results);

    /**
    * Removes a job.
    *
    * Removes a job from storage and frees all stored results.
    *
    * @param job_id the id of the job
    */
    virtual void remove_job(const gspc::job_id_t& job_id);

    /**
    * Returns the number of the expected results.
    *
    * @param job_id the job_id to lookup the expected results for
    * @return       the number of expected results
    */
    virtual unsigned get_number_of_expected_results(const gspc::job_id_t& job_id) const;

    /**
    * Returns the number of the received results.
    *
    * @param job_id the job_id to lookup the received results for
    * @return       the number of received results
    */
    virtual unsigned get_number_of_received_results(const gspc::job_id_t& job_id) const;

    /**
    * Adds a result to the storage.
    *
    * @param job_id      the id of the job to be added to the storage
    * @param task_result the result of the executed task
    */
    virtual void push(const gspc::job_id_t& job_id, task_result&& task_result);

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
    virtual optional_result get(const gspc::job_id_t& job_id) const;

    /**
    * Removes the earliest result from the storage.
    *
    * @param job_id the id of the job to remove the result from
    */
    virtual void pop(const gspc::job_id_t& job_id);
  private:
    boost::python::object _object;
  };
}