#define PY_SSIZE_T_CLEAN

#include "workflow/handle_error.hpp"
#include "workflow/load_worker_config.hpp"

#include <dlfcn.h>
#include <fstream>
#include <sstream>

void last_dl_error_msg (std::string* pError)
{
  const char* msg = ::dlerror();
  if (msg != NULL)
    pError->assign (msg);
  else
    pError->assign ("(Unknown error)");
}

bool load_library
  (const std::string& libPath, void** ppLib, std::string* pError)
{
  *ppLib = NULL;
  *ppLib = ::dlopen (libPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if (*ppLib == NULL)
  {
    last_dl_error_msg(pError);
    *pError = libPath + " - " + *pError;
    return false;
  }
  else
  {
    return true;
  }
}

bool close_library(void* pLib, std::string* pError)
{
  if (::dlclose(pLib) != 0)
  {
    last_dl_error_msg (pError);
    return false;
  }
  else
  {
    return true;
  }
}

#define LOAD_PYTHON_SYMBOL(name) \
  dart::name = (dart::name##_t*)::dlsym (pLib, #name); \
  if (dart::name == NULL) \
  { \
    last_dl_error_msg (pError); \
    *pError = "Error: " + *pError; \
    return false; \
  } \

bool load_symbols (void* pLib, std::string* pError)
{
  LOAD_PYTHON_SYMBOL (Py_Initialize)
  LOAD_PYTHON_SYMBOL (Py_IsInitialized)
  LOAD_PYTHON_SYMBOL (Py_Finalize)
  LOAD_PYTHON_SYMBOL (Py_DecodeLocale)
  LOAD_PYTHON_SYMBOL (Py_SetPythonHome)
  LOAD_PYTHON_SYMBOL (PyRun_SimpleStringFlags)
  LOAD_PYTHON_SYMBOL (PyUnicode_DecodeFSDefault)
  LOAD_PYTHON_SYMBOL (PyImport_Import)
  LOAD_PYTHON_SYMBOL (Py_DecRef)
  LOAD_PYTHON_SYMBOL (PyObject_GetAttrString)
  LOAD_PYTHON_SYMBOL (PyCallable_Check)
  LOAD_PYTHON_SYMBOL (PyUnicode_FromString)
  LOAD_PYTHON_SYMBOL (PyObject_CallFunctionObjArgs)
  LOAD_PYTHON_SYMBOL (PyErr_Occurred)
  LOAD_PYTHON_SYMBOL (PyObject_GetAttrString)
  LOAD_PYTHON_SYMBOL (PyUnicode_AsEncodedString)
  LOAD_PYTHON_SYMBOL (PyBytes_AsString)
  LOAD_PYTHON_SYMBOL (PyObject_GetBuffer)
  LOAD_PYTHON_SYMBOL (PyBuffer_Release)
  LOAD_PYTHON_SYMBOL (PyErr_NormalizeException)
  LOAD_PYTHON_SYMBOL (PyErr_Fetch)
  LOAD_PYTHON_SYMBOL (PyObject_Repr)
  LOAD_PYTHON_SYMBOL (PyImport_ImportModule)
  LOAD_PYTHON_SYMBOL (PyObject_CallObject)
  LOAD_PYTHON_SYMBOL (PyUnicode_AsASCIIString)
  LOAD_PYTHON_SYMBOL (PyErr_Print)
  LOAD_PYTHON_SYMBOL (Py_BuildValue)
  LOAD_PYTHON_SYMBOL (PySys_GetObject)
  LOAD_PYTHON_SYMBOL (PyObject_HasAttrString)
  LOAD_PYTHON_SYMBOL (PyObject_SetAttrString)
  LOAD_PYTHON_SYMBOL (PyArg_ParseTuple)
  LOAD_PYTHON_SYMBOL (PyCFunction_NewEx)

  return true;
}

namespace 
{
  static std::ofstream logfile;
}

static PyObject* capture_stdout(PyObject* self, PyObject* args)
{
  char* string;
  if (dart::PyArg_ParseTuple(args, "s", &string))
  {
    logfile << "[Python stdout] " << string << std::endl;
  }

  return dart::Py_BuildValue("");
}

static PyObject* capture_stderr(PyObject* self, PyObject* args)
{
  char* string;
  if (dart::PyArg_ParseTuple(args, "s", &string))
  {
    logfile << "[Python stderr] " << string << std::endl;
  }

  return dart::Py_BuildValue("");
}

std::pair<std::string, std::vector<char>> run_python_task
  ( std::string const& python_home
  , std::string const& python_library
  , std::string const& path_to_module
  , std::string const& method
  , std::string const& method_params
  , std::string const& //worker
  , std::string const& log_file
  )
{
  logfile = std::ofstream(log_file, std::ios::app);
  logfile << std::endl;

  void* pLib (nullptr);
  std::string error;

  if (!load_library (python_library, &pLib, &error))
  {
    std::string err_msg ("Could not open  the library \""
			            + python_library
			            + "\": "
			            + error
			            );
    logfile << err_msg << std::endl;
    return std::make_pair (err_msg, std::vector<char>());
  }

  if (!load_symbols (pLib, &error))
  {
    logfile << error << std::endl;
    return std::make_pair (error, std::vector<char>());
  }
  
  PyObject *pModule, *pFunc;

  if (dart::Py_IsInitialized() == 0) // not initialized
  {
    logfile << "Initializing Python" << std::endl;
    wchar_t* pyhome = dart::Py_DecodeLocale(python_home.c_str(), NULL);

    dart::Py_SetPythonHome(pyhome);
    dart::Py_Initialize();
  }

  std::string path = "";
  std::string module = "";
  {
    auto last = path_to_module.find_last_of("/");
    path = dart::worker_config.module_prefix + path_to_module.substr(0, last + 1);
    module = path_to_module.substr(last + 1);
  }
  logfile << "using module (" << module << ") with path (" << path << ")" << std::endl;
  dart::PyRun_SimpleStringFlags("import sys", NULL);
  std::ostringstream osstr;
  osstr << "if not '" << path << "' in sys.path: sys.path.append ('" << path << "')";
  if (dart::PyRun_SimpleStringFlags(osstr.str().c_str(), NULL) < 0)
    return handle_error();

  // Catch stdout and stderr
  {
    {
      std::ostringstream stream;
      stream
        << "class __dart_out :\n"
        << "  def __init__(self) :\n"
        << "    pass\n"
        << "  def write(self, txt) :\n"
        << "    pass\n"
        << "  def flush(self) :\n"
        << "    pass\n"
        << "if sys.stderr == None:\n"
        << "  sys.stderr = __dart_out()\n"
        << "if sys.stdout == None:\n"
        << "  sys.stdout = __dart_out()\n";
      if (dart::PyRun_SimpleStringFlags(stream.str().c_str(), NULL) < 0)
        return handle_error();
    }

    {
      PyCFunction func = capture_stdout;
      PyMethodDef method = { "capture_stdout_function", func, METH_VARARGS, "" };
      PyObject* log_capture = dart::PyCFunction_NewEx(&method, NULL, NULL);

      auto* std_obj = dart::PySys_GetObject("stdout");
      if (std_obj != nullptr)
      {
        if (dart::PyObject_SetAttrString(std_obj, "write", log_capture) == -1)
        {
          return handle_error();
        }
      }
    }

    {
      PyCFunction func = capture_stderr;
      PyMethodDef method = PyMethodDef{ "capture_stderr_function", func, METH_VARARGS, "" };
      PyObject* log_capture = dart::PyCFunction_NewEx(&method, NULL, NULL);
      auto* std_obj = dart::PySys_GetObject("stderr");
      if (std_obj != nullptr)
      {
        if (dart::PyObject_SetAttrString(std_obj, "write", log_capture) == -1)
        {
          return handle_error();
        }
      }
    }
  }
  
  pModule = dart::PyImport_ImportModule(module.c_str());
 
  if (pModule != NULL)
  {
    pFunc = dart::PyObject_GetAttrString(pModule, method.c_str());

    if (pFunc && dart::PyCallable_Check (pFunc))
    {
      PyObject* params = dart::PyUnicode_FromString (method_params.c_str());
      PyObject* res (dart::PyObject_CallFunctionObjArgs (pFunc, params, NULL));
      if (!res)
      {
        std::string err_msg;

        if (dart::PyErr_Occurred())
        {
          err_msg = handle_error().first;
        }

        logfile << "Python Error: " << err_msg << std::endl;

        return std::make_pair (err_msg, std::vector<char>());
      }
      
      Py_buffer view;
      if (dart::PyObject_GetBuffer (res, &view, PyBUF_C_CONTIGUOUS | PyBUF_SIMPLE) < 0)
      {
        return handle_error();
      }

      std::vector<char> pyres ((char*)view.buf, (char*)view.buf + view.len);

      dart::PyBuffer_Release (&view);
      dart::Py_DecRef (res);

      dart::Py_DecRef (pFunc);
      dart::Py_DecRef (pModule);

      return std::make_pair ("", pyres);
    }
    else
    {
      dart::Py_DecRef(pFunc);
      dart::Py_DecRef(pModule);

      return handle_error();
    }
  }
  else
  {
    return handle_error();
  }

  return {};
}
