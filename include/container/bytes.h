#ifndef __CONTAINER_BYTES_H__
#define __CONTAINER_BYTES_H__ (1)

#include <object.h>

namespace cppbind {

struct Bytes {
  Object obj;

  Bytes(void) : obj(PyBytes_FromString("")) {}
  Bytes(const Object &other) : obj(other) {}
  Bytes(PyObject *pyobj) : obj(nullptr) {
    if (PyBytes_Check(pyobj)) {
      obj.ptr = pyobj;
    } else if (PyUnicode_Check(pyobj)) {
      obj.ptr = PyUnicode_AsUTF8String(pyobj);
    } else {
      cppbind_check_internal(false && "Object is not a bytes object.");
    }
  }
  Bytes(const char *s) : obj(PyBytes_FromString(s)) {}

  char *data() { return PyBytes_AsString(obj.ptr); }
  const char *data() const { return PyBytes_AsString(obj.ptr); }

  char operator[](Py_ssize_t index) const { return data()[index]; }

  Object &object() { return this->obj; }
  const Object &object() const { return this->obj; }

  Py_ssize_t size() const { return PyBytes_Size(obj.ptr); }
};

} /* namespace cppbind */

#endif /* __CONTAINER_BYTES_H__ */
