#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>

namespace dart
{
  typedef PyObject* Py_Initialize_t();
  typedef void Py_Finalize_t();
  typedef wchar_t* Py_DecodeLocale_t (const char*, char*);
  typedef void Py_SetPythonHome_t (wchar_t*);
  typedef void PyRun_SimpleStringFlags_t (const char*, char*);
  typedef PyObject* PyUnicode_DecodeFSDefault_t (const char*);
  typedef PyObject* PyObject_Repr_t (PyObject*);
  typedef PyObject* Py_BuildValue_t (const char*, ...);
  typedef void Py_DecRef_t (PyObject*);
  typedef PyObject* PyImport_Import_t (PyObject*);
  typedef PyObject* PyImport_ImportModule_t (const char*);
  typedef PyObject* PyObject_GetAttrString_t (PyObject*, const char*);
  typedef PyObject* PyUnicode_FromString_t (const char*);
  typedef PyObject* PyUnicode_AsEncodedString_t (PyObject*, const char*, const char*);
  typedef PyObject* PyUnicode_AsASCIIString_t (PyObject*);
  typedef char* PyBytes_AsString_t (PyObject*);
  typedef int PyCallable_Check_t (PyObject*);
  typedef PyObject* PyObject_CallFunctionObjArgs_t (PyObject*, ...);
  typedef PyObject* PyObject_CallObject_t (PyObject*, PyObject*);
  typedef int PyObject_GetBuffer_t (PyObject*, Py_buffer*, int);
  typedef void PyBuffer_Release_t (Py_buffer*);
  typedef PyObject* PyErr_Occurred_t();
  typedef void PyErr_NormalizeException_t (PyObject**, PyObject**, PyObject**);
  typedef void PyErr_Fetch_t (PyObject**, PyObject**, PyObject**);
  typedef void PyErr_Print_t();

  Py_Initialize_t* Py_Initialize (nullptr);
  Py_Finalize_t* Py_Finalize(nullptr);
  Py_DecodeLocale_t* Py_DecodeLocale (nullptr);
  Py_SetPythonHome_t* Py_SetPythonHome (nullptr);
  PyRun_SimpleStringFlags_t* PyRun_SimpleStringFlags (nullptr);
  PyUnicode_DecodeFSDefault_t* PyUnicode_DecodeFSDefault (nullptr);
  PyObject_Repr_t* PyObject_Repr (nullptr);
  Py_BuildValue_t* Py_BuildValue (nullptr);
  Py_DecRef_t* Py_DecRef (nullptr);
  PyImport_Import_t* PyImport_Import (nullptr);
  PyImport_ImportModule_t* PyImport_ImportModule (nullptr);
  PyObject_GetAttrString_t* PyObject_GetAttrString (nullptr);
  PyUnicode_FromString_t* PyUnicode_FromString (nullptr);
  PyUnicode_AsEncodedString_t* PyUnicode_AsEncodedString (nullptr);
  PyUnicode_AsASCIIString_t* PyUnicode_AsASCIIString (nullptr);
  PyBytes_AsString_t* PyBytes_AsString (nullptr);
  PyCallable_Check_t* PyCallable_Check (nullptr);
  PyObject_CallFunctionObjArgs_t* PyObject_CallFunctionObjArgs (nullptr);
  PyObject_CallObject_t* PyObject_CallObject (nullptr);
  PyObject_GetBuffer_t* PyObject_GetBuffer (nullptr);
  PyBuffer_Release_t* PyBuffer_Release (nullptr);
  PyErr_Occurred_t* PyErr_Occurred (nullptr);
  PyErr_NormalizeException_t* PyErr_NormalizeException (nullptr);
  PyErr_Fetch_t* PyErr_Fetch (nullptr);
  PyErr_Print_t* PyErr_Print (nullptr);
}
