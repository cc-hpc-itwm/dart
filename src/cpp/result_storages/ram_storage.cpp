#include "cpp/result_storages/ram_storage.hpp"

using namespace result_storages;

/**
* Adds a job to the storage.
*
* Adds a job to the storage and saves number of total expected results.
*
* @param job_id           the id of the job
* @param expected_results the number of expected results for this job
*/
void ram_storage::add_job(const gspc::job_id_t& job_id, unsigned expected_results)
{
  _storage[job_id] = { std::queue<task_result>(), expected_results, 0 };
}

/**
* Removes a job.
*
* Removes a job from storage and frees all stored results.
*
* @param job_id the id of the job
*/
void ram_storage::remove_job(const gspc::job_id_t& job_id)
{
  _storage.erase(job_id);
}

/**
* Returns the number of the expected results.
*
* @param job_id the job_id to lookup the expected results for
* @return       the number of expected results
*/
unsigned ram_storage::get_number_of_expected_results(const gspc::job_id_t& job_id) const
{
  auto iter = _storage.find(job_id);
  if (iter == _storage.end())
    return 0;
  return iter->second.expected_results;
}

/**
* Returns the number of the received results.
*
* @param job_id the job_id to lookup the received results for
* @return       the number of received results
*/
unsigned ram_storage::get_number_of_received_results(const gspc::job_id_t& job_id) const
{
  auto iter = _storage.find(job_id);
  if (iter == _storage.end())
    return 0;
  return iter->second.received_results;
}

/**
* Adds a result to the storage.
*
* @param job_id      the id of the job to be added to the storage
* @param task_result the result of the executed task
*/
void ram_storage::push(const gspc::job_id_t& job_id, task_result&& task_result)
{
  ++(_storage[job_id].received_results);
  _storage[job_id].results.emplace(task_result);
}

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
ram_storage::optional_result ram_storage::get(const gspc::job_id_t& job_id) const
{
  auto iter = _storage.find(job_id);
  if (iter == _storage.end())
    return boost::none;
  
  if (iter->second.results.empty())
    return boost::none;

  return iter->second.results.front();
}

/**
* Removes the earliest result from the storage.
*
* @param job_id the id of the job to remove the result from
*/
void ram_storage::pop(const gspc::job_id_t& job_id)
{
  auto iter = _storage.find(job_id);
  if (iter == _storage.end())
    return;
  
  iter->second.results.pop();
  if (iter->second.results.empty() && (iter->second.received_results >= iter->second.expected_results))
  {
    // There will probably no more results coming, so we can delete the entry for this job
    _storage.erase(iter);
  }
}