#include <cpp/task_result.hpp>

#include <string>

boost::python::dict task_result_to_python(const task_result& result) noexcept
{
  boost::python::dict dict_res;

  dict_res["task_id"] = result.task_id;
  dict_res["location"] = result.location;
  dict_res["host"] = result.host;
  dict_res["worker"] = result.worker;
  dict_res["start_time"] = result.start_time;
  dict_res["duration"] = result.duration;

  if (!result.error.empty())
  {
    dict_res["error"] = result.error;
  }
  else
  {
    dict_res["result"] = result.success.to_string();
  }

  return dict_res;
}

task_result task_result_from_python(const boost::python::dict& pyresult)
{
  task_result res;
  res.task_id = boost::python::extract<std::string>(pyresult["task_id"]);
  res.location = boost::python::extract<std::string>(pyresult["location"]);
  res.host = boost::python::extract<std::string>(pyresult["host"]);
  res.worker = boost::python::extract<std::string>(pyresult["worker"]);
  res.start_time = boost::python::extract<std::string>(pyresult["start_time"]);
  res.duration = boost::python::extract<float>(pyresult["duration"]);

  if (!pyresult.has_key("result"))
  {
    res.error = boost::python::extract<std::string>(pyresult["error"]);
  }
  else
  {
    // Copy string contents before passing it to the byte array
    std::string s = boost::python::extract<std::string>(pyresult["result"]);
    res.success = we::type::bytearray(s);
  }

  return res;
}