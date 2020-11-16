#include <cpp/drts_interface.hpp>
#include <boost/python/overloads.hpp>

void translate (std::exception const& e)
{
  PyErr_SetString (PyExc_RuntimeError, e.what());
}

boost::python::list (drts_wrapper::*exec_0)
  ( std::string const& method
  , boost::python::list const& locations_and_parameters
  ) = &drts_wrapper::run;

boost::python::list (drts_wrapper::*exec_1)
  ( std::string const& method
  , boost::python::list const& locations_and_parameters
  , std::string const& output_directory
  ) = &drts_wrapper::run;

boost::python::list (drts_wrapper::*exec_2)
  ( std::string const& abs_path_to_module
  , std::string const& method
  , boost::python::list const& locations_and_parameters
  ) = &drts_wrapper::run;

boost::python::list (drts_wrapper::*exec_3)
  ( std::string const& abs_path_to_module
  , std::string const& method
  , boost::python::list const& locations_and_parameters
  , std::string const& output_directory
  ) = &drts_wrapper::run;

gspc::job_id_t (drts_wrapper::*async_exec_0)
  ( std::string const& method
  , boost::python::list const& locations_and_parameters
  ) = &drts_wrapper::async_run;

gspc::job_id_t (drts_wrapper::*async_exec_1)
  ( std::string const& method
  , boost::python::list const& locations_and_parameters
  , std::string const& output_directory
  ) = &drts_wrapper::async_run;

gspc::job_id_t (drts_wrapper::*async_exec_2)
  ( std::string const& abs_path_to_module
  , std::string const& method
  , boost::python::list const& locations_and_parameters
  ) = &drts_wrapper::async_run;

gspc::job_id_t (drts_wrapper::*async_exec_3)
  ( std::string const& abs_path_to_module
  , std::string const& method
  , boost::python::list const& locations_and_parameters
  , std::string const& output_directory
  ) = &drts_wrapper::async_run;

void (drts_wrapper::*start_runtime_0)() = &drts_wrapper::start_runtime;
void (drts_wrapper::*start_runtime_1) (std::string const&) = &drts_wrapper::start_runtime;
void (drts_wrapper::*start_runtime_2) (std::string const&, std::size_t) = &drts_wrapper::start_runtime;

void (drts_wrapper::*add_workers_0) (std::string const&) = &drts_wrapper::add_workers;
void (drts_wrapper::*add_workers_1)
  ( std::string const&
  , std::size_t
  , boost::python::list const&
  , std::size_t
  ) = &drts_wrapper::add_workers;
void (drts_wrapper::*add_workers_2)
  ( boost::python::list const& pyhosts
  , std::size_t workers_per_host
  , boost::python::list const& capabilities
  , std::size_t shm_size
  ) = &drts_wrapper::add_workers;

boost::python::dict (drts_wrapper::*remove_workers_0)() = &drts_wrapper::remove_workers;
boost::python::dict (drts_wrapper::*remove_workers_1)
  (boost::python::list const&) = &drts_wrapper::remove_workers;

BOOST_PYTHON_MODULE (DART)
{
  using namespace boost::python;
  register_exception_translator<std::exception>(&translate);

  class_<drts_wrapper, boost::noncopyable>
    ("runtime"
    , init<std::string, boost::python::dict>()
    )
    .def("start", start_runtime_0)
    .def("start", start_runtime_1)
    .def("start", start_runtime_2)
    .def("add_workers", add_workers_0)
    .def("add_workers", add_workers_1)
    .def("add_workers", add_workers_2)
    .def("remove_workers", remove_workers_0)
    .def("remove_workers", remove_workers_1)
    .def("run", exec_0)
    .def("run", exec_1)
    .def("run", exec_2)
    .def("run", exec_3)
    .def("async_run", async_exec_0)
    .def("async_run", async_exec_1)
    .def("async_run", async_exec_2)
    .def("async_run", async_exec_3)
    .def("collect_results", &drts_wrapper::collect_results)
    .def("get_number_of_remaining_tasks", &drts_wrapper::get_number_of_remaining_tasks)
    .def("get_total_number_of_tasks", &drts_wrapper::get_total_number_of_tasks)
    .def("is_result_available", &drts_wrapper::is_result_available)
    .def("pop_result", &drts_wrapper::pop_result)
    .def("stop", &drts_wrapper::stop_runtime)
    .def("__exit__", &drts_wrapper::stop_runtime)
    ;
}
