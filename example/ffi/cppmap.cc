/**
 * CppMap implementation.
 */
#include "ffi.h"

using cppbind::Object;
using example::CppMap;

PyObject *CppMap::get(const ::cppbind::Tuple &args) const {
  if (args.size() == 0) {
    PyErr_SetString(PyExc_ValueError, "key cannot be empty");
    return nullptr;
  }

  Object key = args[0];
  if (!comparable(key.ptr)) {
    return nullptr;
  }

  PyObject *default_value = args.size() > 1 ? (args[1].ptr) : Py_None;

  auto it = table.find(key.ptr);
  if (it == table.end()) {
    return default_value;
  }
  return it->second;
}

PyObject *CppMap::put(const ::cppbind::Tuple &args) {
  if (args.size() != 2) {
    PyErr_SetString(PyExc_ValueError,
                    "requires exactly 2 arguments: key and value");
    return nullptr;
  }

  Object key = args[0];
  Object value = args[1];
  if (!comparable(key.ptr)) {
    return nullptr;
  }

  (void)put(key.ptr, value.ptr);
  return Py_None;
}

extern "C" PyObject *CppMap_New(PyObject *self, PyObject *compare_fn) {
  auto *type = cppbind::Type<CppMap>::instance;
  PyObject *ret = _PyObject_New((PyTypeObject *)type);
  new (ret) CppMap(compare_fn);
  return ret;
}
