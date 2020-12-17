#define PY_SSIZE_T_CLEAN

#include "workflow/handle_error.hpp"

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

struct on_scope_exit_finalize_python_and_close_library
{
  void* _lib;
  on_scope_exit_finalize_python_and_close_library(void* pLib) : _lib(pLib) { }
  ~on_scope_exit_finalize_python_and_close_library() { dart::Py_Finalize(); std::string _; close_library(_lib, &_); }
};

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

  return true;
}

std::pair<std::string, std::vector<char>> run_python_task
  ( std::string const& python_home
  , std::string const& python_library
  , std::string const& path_to_module_or_module_content
  , std::string const& is_path
  , std::string const& method
  , std::string const& method_params
  , std::string const& //worker
  , std::string const& log_file
  )
{
  std::ofstream log(log_file);
  void* pLib (nullptr);
  std::string error;

  log << "loading library" << std::endl;
  if (!load_library (python_library, &pLib, &error))
  {
    std::string err_msg ("Could not open  the library \""
			            + python_library
			            + "\": "
			            + error
			            );
    return std::make_pair (err_msg, std::vector<char>());
  }

  log << "loading symbols" << std::endl;
  if (!load_symbols (pLib, &error))
  {
    return std::make_pair (error, std::vector<char>());
  }

  on_scope_exit_finalize_python_and_close_library scope_guard(pLib);

  PyObject *pModule, *pFunc;

  log << "setting home" << std::endl;
  wchar_t *pyhome = dart::Py_DecodeLocale (python_home.c_str(), NULL);

  dart::Py_SetPythonHome (pyhome);
  dart::Py_Initialize();

  std::string path = "";
  std::string module = "";
  if (is_path == "true")
  {
    auto last = path_to_module_or_module_content.find_last_of("/");
    path = path_to_module_or_module_content.substr(0, last + 1);
    module = path_to_module_or_module_content.substr(last + 1);
  }
  else
  {
    path = ".";
    std::ofstream mod("mod.py");
    mod << path_to_module_or_module_content;
    mod.close();
    module = "mod.py";
  }
  log << "using module (" << module << ") with path (" << path << ")" << std::endl;
  dart::PyRun_SimpleStringFlags("import sys", NULL);
  std::ostringstream osstr;
  osstr << "sys.path.append ('" << path << "')";
  dart::PyRun_SimpleStringFlags(osstr.str().c_str(), NULL);
  pModule = dart::PyImport_ImportModule(module.c_str());
 
  if (pModule != NULL)
  {
    pFunc = dart::PyObject_GetAttrString(pModule, method.c_str());

    if (pFunc && dart::PyCallable_Check (pFunc))
    {
      log << method_params << std::endl;
      PyObject* params = dart::PyUnicode_FromString (method_params.c_str());

      std::ofstream ofslog (log_file.c_str(), std::ofstream::app);
      ofslog << std::endl;

      PyObject* res (dart::PyObject_CallFunctionObjArgs (pFunc, params, NULL));
      if (!res)
      {
        std::string err_msg;

        if (dart::PyErr_Occurred())
        {
          err_msg = handle_error().first;
        }

        PyObject* catch_err = dart::PyObject_GetAttrString (pModule,"catch_stderr");

        if (catch_err)
        {
          dart::PyErr_Print();

          PyObject* output = dart::PyObject_GetAttrString (catch_err,"value");
          PyObject* pyStr = dart::PyUnicode_AsEncodedString (output, "utf-8","Error ~");

          err_msg += (dart::PyBytes_AsString (pyStr));

          dart::Py_DecRef (output);
          dart::Py_DecRef (pyStr);
          dart::Py_DecRef (catch_err);
          dart::Py_DecRef (params);
          dart::Py_DecRef (pFunc);
          dart::Py_DecRef (pModule);
        }

        ofslog << "Error: " << err_msg << std::endl;

        return std::make_pair (err_msg, std::vector<char>());
      }

      PyObject* catch_out = dart::PyObject_GetAttrString (pModule,"catch_stdout");
      if (catch_out)
      {
        PyObject* output = dart::PyObject_GetAttrString (catch_out,"value");
        PyObject* pyStr = dart::PyUnicode_AsEncodedString (output, "utf-8","Error ~");

        std::ostringstream osstr;
        osstr << dart::PyBytes_AsString (pyStr);
        if (!osstr.str().empty())
        {
          ofslog << "Logging info: \n" << osstr.str() << std::endl;
        }

        dart::Py_DecRef (output);
        dart::Py_DecRef (pyStr);
        dart::Py_DecRef (catch_out);
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

  close_library (pLib, &error);

  return {};
}
