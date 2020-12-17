#include "workflow/python3_functions.hpp"

#include <string>
#include <utility>
#include <vector>

std::pair<std::string, std::vector<char>> handle_error()
{
  PyObject *pytype, *pyvalue, *pytraceback;

  dart::PyErr_Fetch (&pytype, &pyvalue, &pytraceback);
  dart::PyErr_NormalizeException (&pytype, &pyvalue, &pytraceback);

  PyObject* strtype = dart::PyObject_Repr (pytype);
  PyObject* pytypestr = dart::PyUnicode_AsEncodedString (strtype, "utf-8", "Error ~");
  std::string err_msg (dart::PyBytes_AsString (pytypestr));

  PyObject* strvalue = dart::PyObject_Repr (pyvalue);
  PyObject* pyvaluestr = dart::PyUnicode_AsEncodedString (strvalue, "utf-8", "Error ~");
  err_msg += ":";
  err_msg += dart::PyBytes_AsString (pyvaluestr);

  if (pytraceback)
  {
    std::string f_get_traceback = "";
    f_get_traceback += "def get_pretty_traceback(exc_type, exc_value, exc_tb):\n";
    f_get_traceback += "    import sys, traceback\n";
    f_get_traceback += "    lines = []\n";
    f_get_traceback += "    lines = traceback.format_exception(exc_type, exc_value, exc_tb)\n";
    f_get_traceback += "    output = '\\n'.join(lines)\n";
    f_get_traceback += "    return output\n";

    dart::PyRun_SimpleStringFlags (f_get_traceback.c_str(), NULL);
    PyObject* mod = dart::PyImport_ImportModule ("__main__");
    PyObject* method = dart::PyObject_GetAttrString (mod, "get_pretty_traceback");
    PyObject* outStr = dart::PyObject_CallObject (method, dart::Py_BuildValue ("OOO", pytype, pyvalue, pytraceback));
    err_msg += dart::PyBytes_AsString (dart::PyUnicode_AsASCIIString (outStr));

    dart::Py_DecRef (mod);
    dart::Py_DecRef (method);
    dart::Py_DecRef (outStr);
  }

  std::pair<std::string, std::vector<char>> result
    {std::move (err_msg), std::vector<char>()};

  dart::Py_DecRef (strtype);
  dart::Py_DecRef (pytypestr);

  dart::Py_DecRef (strvalue);
  dart::Py_DecRef (pyvaluestr);

  dart::Py_DecRef (pytype);
  dart::Py_DecRef (pyvalue);
  dart::Py_DecRef (pytraceback);

  return result;
}
