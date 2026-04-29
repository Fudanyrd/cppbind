#ifndef __CONTAINER_BYTES_H__
#define __CONTAINER_BYTES_H__ (1)

#include <object.h>

namespace cppbind {

/**
 * Python's bytes type without item assignment.
 */
struct Bytes {
private:
  Object obj;

public:
  /**
   * Default constructor. It will create an empty bytes
   * object.
   */
  Bytes(void) : obj(PyBytes_FromString("")) {}

  /**
   * Construct from {@link Object} <b>without</b> checking
   * whether the object is a bytes object.
   */
  Bytes(const Object &other) : obj(other) {}

  /**
   * Initialize from a Python object. It will check whether the object is a
   * bytes object or a unicode object. If it is a bytes object, it will directly
   * use it. If it is a unicode object, it will convert it to a bytes object via
   * UTF-8 encoding (i.e. Python's `.encode("utf-8")`).
   *
   * @throw AssertionError if the object is neither a bytes object
   * nor a unicode object.
   */
  Bytes(PyObject *pyobj) : obj(nullptr) {
    if (PyBytes_Check(pyobj)) {
      obj.ptr = pyobj;
    } else if (PyUnicode_Check(pyobj)) {
      obj.ptr = PyUnicode_AsUTF8String(pyobj);
    } else {
      cppbind_check_internal(false && "Object is not a bytes object.");
    }
  }
  /**
   * Equivalent to python code:
   * <blockquote><pre>thisBytes = s.encode()
   * </pre></blockquote>
   */
  Bytes(const char *s) : obj(PyBytes_FromString(s)) {}

  /**
   * @return the pointer to the bytes data.
   */
  char *data() { return PyBytes_AsString(obj.ptr); }

  /**
   * @return the pointer to the bytes data.
   */
  const char *data() const { return PyBytes_AsString(obj.ptr); }

  /**
   * Equivalent to python code:
   * <blockquote><pre>thisBytes[index]
   * </pre></blockquote>
   */
  char operator[](Py_ssize_t index) const { return data()[index]; }

  /**
   * @return the reference to current object.
   */
  Object &object() { return this->obj; }
  /**
   * @return the reference to current object.
   */
  const Object &object() const { return this->obj; }

  /**
   * @return the size of the bytes object.
   */
  Py_ssize_t size() const { return PyBytes_Size(obj.ptr); }
};

} /* namespace cppbind */

#endif /* __CONTAINER_BYTES_H__ */
