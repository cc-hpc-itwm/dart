#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <cpp/task_interface.hpp>
#include <iostream>

BOOST_AUTO_TEST_CASE (module_not_found)
{
  boost::filesystem::path py_exe (PYTHON_EXECUTABLE);

  auto const result = run_python_task
    ( py_exe.parent_path().parent_path().string()
    , PYTHON_LIBRARIES
    , "."
    , "exceptions"
    , "module_not_found"
    , ""
    , "worker"
    , "/var/tmp/log.txt"
    );

  BOOST_REQUIRE (!result.first.empty());

  BOOST_REQUIRE
    (result.first.find ("ModuleNotFoundError") != std::string::npos);
}

BOOST_AUTO_TEST_CASE (division_by_zero)
{
  boost::filesystem::path py_exe (PYTHON_EXECUTABLE);

  auto const result = run_python_task
    ( py_exe.parent_path().parent_path().string()
    , PYTHON_LIBRARIES
    , "."
    , "exceptions"
    , "division_by_zero"
    , ""
    , "worker"
    , "/var/tmp/log.txt"
    );

  BOOST_REQUIRE (!result.first.empty());

  BOOST_REQUIRE
    (result.first.find ("ZeroDivisionError") != std::string::npos);
}

BOOST_AUTO_TEST_CASE (file_not_found)
{
  boost::filesystem::path py_exe (PYTHON_EXECUTABLE);

  auto const result = run_python_task
    ( py_exe.parent_path().parent_path().string()
    , PYTHON_LIBRARIES
    , "."
    , "exceptions"
    , "file_not_found"
    , ""
    , "worker"
    , "/var/tmp/log.txt"
    );

  BOOST_REQUIRE (!result.first.empty());

  BOOST_REQUIRE
    (result.first.find ("FileNotFoundError") != std::string::npos);
}


BOOST_AUTO_TEST_CASE (syntax_error)
{
  boost::filesystem::path py_exe (PYTHON_EXECUTABLE);

  auto const result = run_python_task
    ( py_exe.parent_path().parent_path().string()
    , PYTHON_LIBRARIES
    , "."
    , "parsing_errors"
    , "syntax_error"
    , ""
    , "worker"
    , "/var/tmp/log.txt"
    );

  BOOST_REQUIRE (!result.first.empty());

  BOOST_REQUIRE
    (result.first.find ("SyntaxError") != std::string::npos);
}
