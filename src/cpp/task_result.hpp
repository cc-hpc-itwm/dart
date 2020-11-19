#pragma once

#include <pnetc/type/task_result.hpp>
#include <boost/python.hpp>

using task_result = pnetc::type::task_result::task_result;

extern boost::python::dict task_result_to_python(const task_result& result) noexcept;
extern task_result task_result_from_python(const boost::python::dict& pyresult);
