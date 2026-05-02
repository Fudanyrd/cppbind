/**
 * CppMap implementation.
 */
#include "ffi.h"

/**
 * <h3>bugprone-easily-swappable-parameters</h3>
 * Make the `type_init_mapping` macro work.
 *
 * <h3>readability-identifier-naming</h3>
 * `CInt_New` is a CPython function.
 *
 * <h3>readability-identifier-length</h3>
 * For macros like `type_init_integer_ops`, using longer
 * identifiers will not improve readability.
 */
/**
 * NOLINTBEGIN(bugprone-easily-swappable-parameters,
 *             readability-identifier-naming,
 *             readability-identifier-length)
 */
type_static_members_declare(example::CppMap);

using cppbind::Object;
using example::CppMap;

::cppbind::Object CppMap::forward_or_convert(const ::cppbind::Object &arg) {
  if (arg.ptr == nullptr) {
    PyErr_SetString(PyExc_ValueError, "argument cannot be null");
    return ::cppbind::Object(nullptr);
  }
  if (PyObject_TypeCheck(arg.ptr, cppbind::Type<CppMap>::instance)) {
    return arg;
  }
  return ::cppbind::Object(nullptr);
}

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

  auto iter = table.find(key.ptr);
  if (iter == table.end()) {
    return default_value;
  }
  return iter->second;
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
  PyObject *ret = _PyObject_New(type);
  new (ret) CppMap(compare_fn);
  return ret;
}

/**
 * NOLINTEND(bugprone-easily-swappable-parameters,
 *           readability-identifier-naming,
 *           readability-identifier-length)
 */
