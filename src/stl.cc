#include <cppbind.h>
#include <stl.h>

static bool is_python_str(PyObject *obj) { return PyUnicode_Check(obj); }

static std::string from_python_str(PyObject *str) {
  const char *data = reinterpret_cast<const char *>(PyUnicode_DATA(str));
  Py_ssize_t size = PyUnicode_GET_LENGTH(str);

  if (PyUnicode_KIND(str) == PyUnicode_1BYTE_KIND) {
    return std::string(data, size);
  }

  std::string ret;
  PyObject *bytes = PyUnicode_AsUTF8String(str);
  if (likely(bytes != nullptr)) {
    data = PyBytes_AS_STRING(bytes);
    size = PyBytes_GET_SIZE(bytes);
    ret.assign(data, size);
    Py_DECREF(bytes);
  }

  return ret;
}

namespace cppbind {

std::string stringify(PyObject *obj) {
  if (is_python_str(obj)) {
    return from_python_str(obj);
  } else {
    PyObject *str_obj = PyObject_Str(obj);
    if (str_obj == nullptr) {
      return "";
    }
    std::string result = from_python_str(str_obj);
    Py_DECREF(str_obj);
    return result;
  }
}

std::ostream &operator<<(std::ostream &os, PyObject *obj) {
  const char *data;
  Py_ssize_t size;
  if (is_python_str(obj)) {
    data = reinterpret_cast<const char *>(PyUnicode_DATA(obj));
    size = PyUnicode_GET_LENGTH(obj);
    os.write(data, size);
  } else {
    obj = PyObject_Repr(obj);
    data = reinterpret_cast<const char *>(PyUnicode_DATA(obj));
    size = PyUnicode_GET_LENGTH(obj);
    os.write(data, size);
    Py_DECREF(obj);
  }
  return os;
}

} /* namespace cppbind */
