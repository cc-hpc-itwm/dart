#include "cpp/result_storages/python_storage.hpp"

#include <boost/python.hpp>
#include <cpp/task_result.hpp>

using namespace result_storages;

python_storage::python_storage(const boost::python::object& object)
: _object(object)
{
  //TODO: Check whether object is okey
}

/**
* Adds a job to the storage.
*
* Adds a job to the storage and saves number of total expected results.
*
* @param job_id           the id of the job
* @param expected_results the number of expected results for this job
*/
void python_storage::add_job(const gspc::job_id_t& job_id, unsigned expected_results)
{
  _object.attr("add_job")(job_id, expected_results);
}

/**
* Removes a job.
*
* Removes a job from storage and frees all stored results.
*
* @param job_id the id of the job
*/
void python_storage::remove_job(const gspc::job_id_t& job_id)
{
  _object.attr("remove_job")(job_id);
}

/**
* Returns the number of the expected results.
*
* @param job_id the job_id to lookup the expected results for
* @return       the number of expected results
*/
unsigned python_storage::get_number_of_expected_results(const gspc::job_id_t& job_id) const
{
  return boost::python::extract<unsigned>(_object.attr("get_number_of_expected_results")(job_id));
}

/**
* Returns the number of the received results.
*
* @param job_id the job_id to lookup the received results for
* @return       the number of received results
*/
unsigned python_storage::get_number_of_received_results(const gspc::job_id_t& job_id) const
{
  return boost::python::extract<unsigned>(_object.attr("get_number_of_received_results")(job_id));
}

/**
* Adds a result to the storage.
*
* @param job_id      the id of the job to be added to the storage
* @param task_result the result of the executed task
*/
void python_storage::push(const gspc::job_id_t& job_id, task_result&& task_result)
{
  _object.attr("push")(job_id, task_result_to_python(task_result));
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
result_storage::optional_result python_storage::get(const gspc::job_id_t& job_id) const
{
  auto value = _object.attr("get")(job_id);
  if (value.is_none())
    return boost::none;
  return task_result_from_python(boost::python::extract<boost::python::dict>(value));
}

/**
* Removes the earliest result from the storage.
*
* @param job_id the id of the job to remove the result from
*/
void python_storage::pop(const gspc::job_id_t& job_id)
{
  _object.attr("pop")(job_id);
}