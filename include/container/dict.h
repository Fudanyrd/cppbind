#ifndef __CONTAINER_DICT_H__
#define __CONTAINER_DICT_H__ (1)

#include "object.h"

namespace cppbind {

struct Dict {
private:
  static void setitem(PyObject *dict, PyObject *key, const Object &value) {
    int ret = PyDict_SetItem(dict, key, value.ptr);
    cppbind_assert(ret == 0 && "Failed to set item in dict.");
  }

  static Object getitem(PyObject *dict, PyObject *key,
                        PyObject *default_value) {
    auto ret = Object(PyDict_GetItem(dict, key));
    if (ret.ptr == nullptr) {
      ret.ptr = default_value;
    }
    ret.inc_ref(); /* give the object a reference.  */
    return ret;
  }

public:
  Object obj;
  Dict() : obj(PyDict_New()) {}
  ~Dict() = default;

  struct ObjectRef {
    PyObject *dict;
    PyObject *key;

    ObjectRef(PyObject *dict, PyObject *key) : dict(dict), key(key) {}

    ObjectRef &operator=(const Object &value) {
      Dict::setitem(dict, key, value);
      return *this;
    }

    operator Object() const { return Dict::getitem(dict, key, Py_None); }
  };

  Object operator[](const Object &key) const {
    return getitem(obj.ptr, key.ptr, Py_None);
  }

  ObjectRef operator[](const Object &key) {
    return ObjectRef(obj.ptr, key.ptr);
  }

  void __setitem__(const Object &key, const Object &value) {
    setitem(obj.ptr, key.ptr, value);
  }

  Py_ssize_t size(void) const { return PyDict_Size(obj.ptr); }
  Py_ssize_t __len__(void) const { return size(); }

  void clear(void) { PyDict_Clear(obj.ptr); }
  Object &object() { return this->obj; }
  const Object &object() const { return this->obj; }
};

} /* namespace cppbind */

#endif /* __CONTAINER_DICT_H__ */
